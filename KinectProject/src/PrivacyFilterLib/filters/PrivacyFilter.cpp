#include "PrivacyFilter.h"
#include "ogre/OgreScene.h"
#include "viewer/Scene2DPainter.h"
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include "types/MaskFrame.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "viewer/SilhouetteItem.h"
#include "viewer/SkeletonItem.h"
#include "types/MetadataFrame.h"
#include <QDebug>
#include <QThread>
#include <QElapsedTimer>
#include <iostream>
#include <cmath>
#include <QList>
#include <boost/random.hpp>
#include "Utils.h"
#include "ml/KMeans.h"
#include <cmath>
#include "opencv_utils.h"
#include <QLabel>

void PrivacyLib_InitResources()
{
    Q_INIT_RESOURCE(privacylib);
}

namespace dai {

PrivacyFilter::PrivacyFilter()
    : m_glContext(nullptr)
    , m_gles(nullptr)
    , m_initialised(false)
    , m_fboDisplay(nullptr)
    , m_filter(FILTER_DISABLED)
    , m_file("data.csv")
    , m_out(&m_file)
    , m_make_capture(false)
{
    QSurfaceFormat format;
    format.setMajorVersion(2);
    format.setMinorVersion(0);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::SingleBuffer);

    m_surface.setFormat(format);
    m_surface.create();

    if (!m_surface.isValid()) {
        qDebug() << "The surface could not be created";
        throw 1;
    }

    m_scene = new Scene2DPainter;
    m_ogreScene = new OgreScene;

    // haarcascade_frontalface_default
    // haarcascade_frontalface_alt.xml
    if (!m_face_cascade.load("haarcascade_frontalface_alt.xml"))
        qDebug() << "Error loading cascades";
}

PrivacyFilter::~PrivacyFilter()
{
   stopListener();
   freeResources();
   if (m_file.isOpen())
       m_file.close();
   qDebug() << "PrivacyFilter::~PrivacyFilter";
}

void PrivacyFilter::initialise(int width, int height)
{
    m_width = width;
    m_height = height;
    m_glContext = new QOpenGLContext;
    m_glContext->setFormat(m_surface.format());

    if (!m_glContext->create()) {
        qDebug() << "Could not create the OpenGL context";
        throw 1;
    }

    m_glContext->makeCurrent(&m_surface);
    m_gles = m_glContext->functions();
    m_ogreScene->initialise(m_width, m_height);
    m_scene->setAvatarTexture(m_ogreScene->texture());
    m_fboDisplay = ScenePainter::createFBO(m_width, m_height);
    m_scene->initScene(width, height);
    m_glContext->doneCurrent();
    m_initialised = true;
}

void PrivacyFilter::freeResources()
{
    if (m_glContext)
    {
        m_glContext->makeCurrent(&m_surface);

        if (m_ogreScene) {
            delete m_ogreScene;
            m_ogreScene = nullptr;
        }

        if (m_scene) {
            delete m_scene;
            m_scene = nullptr;
        }

        if (m_fboDisplay) {
            m_fboDisplay->release();
            delete m_fboDisplay;
            m_fboDisplay = nullptr;
        }

        m_glContext->doneCurrent();

        delete m_glContext;
        m_glContext = nullptr;
    }

    m_gles = nullptr;
    m_initialised = false;
}

shared_ptr<QHashDataFrames> PrivacyFilter::allocateMemory()
{
    m_frames = make_shared<QHashDataFrames>();
    return m_frames;
}

void PrivacyFilter::resize(int width, int height)
{
    if (width == m_width && height == m_height)
        return;

    m_width = width;
    m_height = height;

    m_glContext->makeCurrent(&m_surface);
    m_ogreScene->resize(m_width, m_height);
    m_scene->setAvatarTexture(m_ogreScene->texture());

    if (m_fboDisplay) {
        delete m_fboDisplay;
        m_fboDisplay = nullptr;
    }

    m_fboDisplay = ScenePainter::createFBO(m_width, m_height);
    m_scene->resize(m_width, m_height);
    m_glContext->doneCurrent();
}

// This method is called from a thread
void PrivacyFilter::newFrames(const QHashDataFrames dataFrames)
{
    if (!m_initialised) {
        initialise();
        FrameGenerator::begin(false);
    }

    // Copy frames (1 ms)
    m_frames->clear();

    foreach (DataFrame::FrameType key, dataFrames.keys()) {
        shared_ptr<DataFrame> frame = dataFrames.value(key);
        m_frames->insert(key, frame->clone());
    }

    // Check if the frames has been copied correctly
    if (!hasExpired()) {
        bool newFrames = generate();
        if (subscribersCount() == 0 || !newFrames) {
            qDebug() << "PrivacyFilter: No listeners or Nothing produced";
            stopListener();
        }
    }
    else {
        qDebug() << "PrivacyFilter - Frame copied out of time";
    }
}

void PrivacyFilter::singleFrame(const QHashDataFrames dataFrames, int width, int height)
{
    //qDebug() << QThread::currentThreadId() << width << height;

    if (!m_initialised) {
        initialise(width, height);
        FrameGenerator::begin(false);
    } else {
        resize(width, height);
    }

    // Copy frames (1 ms)
    m_frames->clear();

    for (DataFrame::FrameType key : dataFrames.keys()) {
        shared_ptr<DataFrame> frame = dataFrames.value(key);
        m_frames->insert(key, frame->clone());
    }

    // Generate
    bool newFrames = generate();

    if (subscribersCount() == 0 || !newFrames) {
        qDebug() << "PrivacyFilter: No listeners or Nothing produced";
        stopListener();
    }
}

void PrivacyFilter::produceFrames(QHashDataFrames &output)
{
    Q_ASSERT(output.contains(DataFrame::Color) && output.contains(DataFrame::Mask));

    ColorFramePtr colorFrame = static_pointer_cast<ColorFrame>(output.value(DataFrame::Color));
    MaskFramePtr maskFrame = static_pointer_cast<MaskFrame>(output.value(DataFrame::Mask));

    // Dilate mask to create a wide border (value = 255)
    shared_ptr<MaskFrame> outputMask = static_pointer_cast<MaskFrame>(maskFrame->clone());
    dilateUserMask(const_cast<uint8_t*>(outputMask->getDataPtr()));

    for (int i=0; i<maskFrame->height(); ++i)
    {
        for (int j=0; j<maskFrame->width(); ++j)
        {
            uint8_t inputValue = maskFrame->getItem(i,j);
            uint8_t outputValue = outputMask->getItem(i,j);

            if (inputValue == 0 && outputValue > 0) {
                maskFrame->setItem(i, j, uint8_t(255));
            }
        }
    }

    //
    // Prepare Scene
    //
    shared_ptr<SilhouetteItem> silhouetteItem = static_pointer_cast<SilhouetteItem>(m_scene->getFirstItem(ITEM_SILHOUETTE));
    shared_ptr<SkeletonItem> skeletonItem = static_pointer_cast<SkeletonItem>(m_scene->getFirstItem(ITEM_SKELETON));

    // Clear items of the scene
    m_scene->clearItems();

    // Background of Scene
    m_scene->setBackground(colorFrame);
    m_scene->setMask(maskFrame);

    // Add silhuette item to the scene
    if (!silhouetteItem)
        silhouetteItem.reset(new SilhouetteItem);

    silhouetteItem->setUser(maskFrame);
    m_scene->addItem(silhouetteItem);

    // Add skeleton item to the scene
    if (output.contains(DataFrame::Skeleton)) {
        if (!skeletonItem)
            skeletonItem.reset(new SkeletonItem);

        skeletonItem->setSkeleton( static_pointer_cast<SkeletonFrame>(output.value(DataFrame::Skeleton)) );
        m_scene->addItem(skeletonItem);
    }

    // Enable Filter
    m_scene->enableFilter(m_filter);

    if (m_filter == FILTER_3DMODEL && output.contains(DataFrame::Skeleton))
        m_ogreScene->enableFilter(true);
    else
        m_ogreScene->enableFilter(false);

    // Prepare Data of the OgreScene
    m_ogreScene->prepareData(output);
    m_scene->markAsDirty();

    //
    // Render scene offscreen
    //

    // Render Avatar
    if (output.contains(DataFrame::Skeleton))
        m_ogreScene->render();

    // Render and compose rest of the scene
    m_glContext->makeCurrent(&m_surface);
    m_scene->resize(m_fboDisplay->width(), m_fboDisplay->height());
    m_scene->renderScene(m_fboDisplay);

    m_fboDisplay->bind();

    // Copy data back to ColorFrame
    /*m_gles->glReadPixels(0,0, m_fboDisplay->width(), m_fboDisplay->height(), GL_RGB, GL_UNSIGNED_BYTE,
                         (GLvoid*) colorFrame->getDataPtr());*/
    convertQImage2ColorFrame(m_fboDisplay->toImage().mirrored(), colorFrame);


    // Face detection
    /*std::vector<cv::Rect> faces = faceDetection(colorFrame);

    cv::Mat color_mat(colorFrame->height(), colorFrame->width(), CV_8UC3,
                      (void*) colorFrame->getDataPtr(), colorFrame->getStride());

    for (size_t i = 0; i < faces.size(); i++) {
        cv::rectangle(color_mat, faces[i], cv::Scalar(255, 255, 0));
    }*/

    // Save color as JPEG
    /*if (m_make_capture) {
        static int capture_id = 1;
        QImage image( (uchar*) colorFrame->getDataPtr(), colorFrame->width(), colorFrame->height(),
                      colorFrame->getStride(), QImage::Format_RGB888);
        image.save("data/capture_" + QString::number(capture_id) + ".jpg");
        capture_id++;
        m_make_capture = false;
    }*/

    m_fboDisplay->release();
    m_glContext->doneCurrent();
}

void PrivacyFilter::afterStop()
{
    freeResources();
    qDebug() << "PrivacyFilter::afterStop";
}

void PrivacyFilter::captureImage()
{
    m_make_capture = true;
}

void PrivacyFilter::enableFilter(ColorFilter filterType)
{
    m_filter = filterType;
}

void PrivacyFilter::dilateUserMask(uint8_t *labels)
{
    int dilationSize = 18;
    cv::Mat newImag(m_height, m_width, cv::DataType<uint8_t>::type, labels);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS,
                                               cv::Size(2*dilationSize + 1, 2*dilationSize+1),
                                               cv::Point( dilationSize, dilationSize ) );
    cv::dilate(newImag, newImag, kernel);
}

std::vector<cv::Rect> PrivacyFilter::faceDetection(shared_ptr<ColorFrame> frame)
{
    Q_ASSERT(frame != nullptr);

    using namespace cv;

    Mat color_mat(frame->height(), frame->width(), CV_8UC3,
                      (void*) frame->getDataPtr(), frame->getStride());

    Mat gray_mat;
    cvtColor(color_mat, gray_mat, CV_RGB2GRAY);
    equalizeHist(gray_mat, gray_mat);
    return faceDetection(gray_mat, true);
}

std::vector<cv::Rect> PrivacyFilter::faceDetection(cv::Mat frameGray, bool equalised)
{
    using namespace cv;
    std::vector<Rect> faces;

    if (!equalised) {
        equalizeHist( frameGray, frameGray );
    }

    // Detect faces
    m_face_cascade.detectMultiScale( frameGray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

    return faces;
}

void PrivacyFilter::convertQImage2ColorFrame(const QImage& input_img, ColorFramePtr output_img)
{
    Q_ASSERT(input_img.width() == output_img->width() && input_img.height() == output_img->height());
    cv::Mat res(input_img.height(), input_img.width(), CV_8UC4, (uchar*) input_img.constBits(), input_img.bytesPerLine());

    for (int i=0; i<res.rows; ++i)
    {
        cv::Vec4b* in_pixel = res.ptr<cv::Vec4b>(i);
        RGBColor* out_pixel = output_img->getRowPtr(i);

        for (int j=0; j<res.cols; ++j)
        {
            out_pixel[j].red = in_pixel[j][2];
            out_pixel[j].green = in_pixel[j][1];
            out_pixel[j].blue = in_pixel[j][0];
        }
    }

    /*QLabel* label = new QLabel;
    label->setPixmap(QPixmap::fromImage(input_img));
    label->show();*/
}

} // End Namespace
