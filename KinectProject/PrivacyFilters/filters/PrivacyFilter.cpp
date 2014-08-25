#include "PrivacyFilter.h"
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

namespace dai {

PrivacyFilter::PrivacyFilter()
    : m_glContext(nullptr)
    , m_gles(nullptr)
    , m_initialised(false)
    , m_fboDisplay(nullptr)
    , m_filter(FILTER_DISABLED)
    , m_file("data.csv")
    , m_out(&m_file)
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
    /*if (!m_face_cascade.load("haarcascade_frontalface_alt.xml"))
        qDebug() << "Error loading cascades";*/
}

PrivacyFilter::~PrivacyFilter()
{
   stopListener();
   freeResources();
   if (m_file.isOpen())
       m_file.close();
   qDebug() << "PrivacyFilter::~PrivacyFilter";
}

void PrivacyFilter::initialise()
{
    m_glContext = new QOpenGLContext;
    m_glContext->setFormat(m_surface.format());

    if (!m_glContext->create()) {
        qDebug() << "Could not create the OpenGL context";
        throw 1;
    }

    m_glContext->makeCurrent(&m_surface);
    m_gles = m_glContext->functions();
    m_ogreScene->initialise();
    m_scene->setAvatarTexture(m_ogreScene->texture());

    // Create frame buffer Object
    QOpenGLFramebufferObjectFormat format;
    format.setInternalTextureFormat(GL_RGB);
    format.setTextureTarget(GL_TEXTURE_2D);
    format.setSamples(0);
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

    m_fboDisplay = new QOpenGLFramebufferObject(QSize(640, 480), format);

    if (!m_fboDisplay->isValid()) {
        qDebug() << "FBO Error";
        throw 2;
    }

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

void PrivacyFilter::produceFrames(QHashDataFrames &output)
{
    Q_ASSERT(output.contains(DataFrame::Color) && output.contains(DataFrame::Mask) &&
             output.contains(DataFrame::Skeleton) && output.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(output.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(output.value(DataFrame::Mask));

    //approach1();
    //approach2();
    //approach3();
    //approach4();
    //approach5();
    //approach6();

    // Dilate mask to create a wide border (value = 255)
    shared_ptr<MaskFrame> outputMask = static_pointer_cast<MaskFrame>(maskFrame->clone());
    dilateUserMask(const_cast<uint8_t*>(outputMask->getDataPtr()));

    for (int i=0; i<maskFrame->getHeight(); ++i)
    {
        for (int j=0; j<maskFrame->getWidth(); ++j)
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
    if (!skeletonItem)
        skeletonItem.reset(new SkeletonItem);

    skeletonItem->setSkeleton( static_pointer_cast<SkeletonFrame>(output.value(DataFrame::Skeleton)) );
    m_scene->addItem(skeletonItem);

    // Enable Filter
    m_scene->enableFilter(m_filter);
    if (m_filter == FILTER_3DMODEL)
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
    m_ogreScene->render();

    // Render and compose rest of the scene
    m_glContext->makeCurrent(&m_surface);
    m_scene->setSize(m_fboDisplay->width(), m_fboDisplay->height());
    m_scene->renderScene(m_fboDisplay);

    m_fboDisplay->bind();

    // Copy data back to ColorFrame
    m_gles->glReadPixels(0,0, m_fboDisplay->width(), m_fboDisplay->height(), GL_RGB, GL_UNSIGNED_BYTE,
                         (GLvoid*) colorFrame->getDataPtr());

    m_fboDisplay->release();
    m_glContext->doneCurrent();
}

void PrivacyFilter::afterStop()
{
    freeResources();
    qDebug() << "PrivacyFilter::afterStop";
}

void PrivacyFilter::enableFilter(ColorFilter filterType)
{
    m_filter = filterType;
}

// Approach 1: log color space (2 channels) without Histogram!!
// Paper: Color Invariants for Person Reidentification
void PrivacyFilter::approach1(QHashDataFrames &frames)
{
    Q_ASSERT(frames.contains(DataFrame::Color) && frames.contains(DataFrame::Mask) &&
             frames.contains(DataFrame::Skeleton) && frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(frames.value(DataFrame::Metadata));

    // Work in the Bounding Box
    if (metadataFrame->boundingBoxes().isEmpty())
        return;

    BoundingBox bb = metadataFrame->boundingBoxes().first();
    shared_ptr<ColorFrame> subColorFrame = colorFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> subMaskFrame = maskFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                             bb.size().width(), bb.size().height());

    // Use cv::Mat for my color frame and mask frame
    cv::Mat inputImg(subColorFrame->getHeight(), subColorFrame->getWidth(),
                 CV_8UC3, (void*)subColorFrame->getDataPtr(), subColorFrame->getStride());

    cv::Mat mask(subMaskFrame->getHeight(), subMaskFrame->getWidth(),
             CV_8UC1, (void*)subMaskFrame->getDataPtr(), subMaskFrame->getStride());

    // Compute Upper and Lower Masks
    cv::Mat upper_mask, lower_mask;
    computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask, mask);

    // Sample points
    const int num_samples = 85;
    QList<Point3b> u_list = randomSampling<uchar, 3>(inputImg, num_samples, upper_mask);
    QList<Point3b> l_list = randomSampling<uchar, 3>(inputImg, num_samples, lower_mask);

    // Convert from RGB color space to 2D log-chromacity color space
    QList<Point2f> log_u_list = convertRGB2Log2DAsList(u_list);
    QList<Point2f> log_l_list = convertRGB2Log2DAsList(l_list);

    // Create a 2D Image of the color space
    float log_range[] = {-5.6f, 5.6f};
    cv::Mat logCoordImg;
    create2DCoordImage({&log_u_list, &log_l_list},
                       {cv::Vec3b(0, 0, 255), cv::Vec3b(255, 0, 0)}, // Red (upper), Blue (lower)
                       logCoordImg,
                       log_range);

    // Show
    cv::imshow("Log.Img", logCoordImg);
    cv::waitKey(1);
}

// Approach 2: CIElab (2 channels)
// Paper: Color Invariants for Person Reidentification
void PrivacyFilter::approach2(QHashDataFrames &frames)
{
    Q_ASSERT(frames.contains(DataFrame::Color) && frames.contains(DataFrame::Mask) &&
             frames.contains(DataFrame::Skeleton) && frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(frames.value(DataFrame::Metadata));

    // Work in the Bounding Box
    if (metadataFrame->boundingBoxes().isEmpty())
        return;

    BoundingBox bb = metadataFrame->boundingBoxes().first();
    shared_ptr<ColorFrame> subColorFrame = colorFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> subMaskFrame = maskFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                             bb.size().width(), bb.size().height());

    // Start OpenCV code
    {using namespace cv;

        // Use cv::Mat for my color frame and mask frame
        Mat inputImg(subColorFrame->getHeight(), subColorFrame->getWidth(),
                     CV_8UC3, (void*)subColorFrame->getDataPtr(), subColorFrame->getStride());

        Mat mask(subMaskFrame->getHeight(), subMaskFrame->getWidth(),
                 CV_8UC1, (void*)subMaskFrame->getDataPtr(), subMaskFrame->getStride());

        // Compute Upper and Lower Masks
        Mat upper_mask, lower_mask;
        computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask, mask); // This ones modifies inputImg

        // Sample Mask
        Mat upper_sampled_mask = upper_mask; //randomSampling<uchar>(upper_mask, 200, upper_mask);
        Mat lower_sampled_mask = lower_mask; //randomSampling<uchar>(lower_mask, 200, lower_mask);

        // Denoise Image
        //denoiseImage(copyImg, copyImg);

        // Convert from RGB color space to CIElab
        // CIElab 8 bits image: all channels range is 0-255.
        Mat imgLab;
        cv::cvtColor(inputImg, imgLab, cv::COLOR_RGB2Lab);

        // Split image in L*, a* and b* channels
        vector<Mat> lab_planes;
        split(imgLab, lab_planes);

        // Create 2D image with only a and b channels
        vector<Mat> ab_planes = {lab_planes[1], lab_planes[2]};
        Mat imgAb;
        merge(ab_planes, imgAb);

        // Compute the histogram for the upper (torso) and lower (leggs) parts
        auto u_hist = Histogram2D<uchar>::create(imgAb, {0, 255}, upper_sampled_mask);
        auto l_hist = Histogram2D<uchar>::create(imgAb, {0, 255}, lower_sampled_mask);

        const int n_often_colors = 8;

        // Create an image of the distribution of the histogram
        Mat histDist;
        createHistDistImage<uchar,2>({u_hist.get(), l_hist.get()},
                                     {Scalar(255, 0, 0), Scalar(0, 0, 255)}, // Blue (upper), Red (lower)
                                     histDist);

        // Create a 2D Image of the histogram
        float ab_range[] = {0.0f, 255.0f};
        Mat histImg;
        create2DCoordImage<uchar>({u_hist.get(), l_hist.get()},
                                  n_often_colors,
                                  {Vec3b(255, 0, 0), Vec3b(0, 0, 255)}, // Blue (upper), Red (lower)
                                  histImg,
                                  ab_range);

        // Modify input image to show used mask
        colorImageWithMask(inputImg, inputImg, upper_mask, lower_mask);

        // Show
        imshow("Hist.Dist", histDist);
        imshow("Hist.Img", histImg);
        waitKey(1);

    } // End OpenCV code

    return;
}

// Approach 3: YUV (2 channels)
// Paper: Color Invariants for Person Reidentification
void PrivacyFilter::approach3(QHashDataFrames& frames)
{
    Q_ASSERT(frames.contains(DataFrame::Color) && frames.contains(DataFrame::Mask) &&
             frames.contains(DataFrame::Skeleton) && frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(frames.value(DataFrame::Metadata));

    // Work in the Bounding Box
    if (metadataFrame->boundingBoxes().isEmpty())
        return;

    BoundingBox bb = metadataFrame->boundingBoxes().first();
    shared_ptr<ColorFrame> subColorFrame = colorFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> subMaskFrame = maskFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                             bb.size().width(), bb.size().height());

    // Start OpenCV code
    {using namespace cv;

        // Use cv::Mat for my color frame and mask frame
        Mat inputImg(subColorFrame->getHeight(), subColorFrame->getWidth(),
                     CV_8UC3, (void*)subColorFrame->getDataPtr(), subColorFrame->getStride());

        Mat mask(subMaskFrame->getHeight(), subMaskFrame->getWidth(),
                 CV_8UC1, (void*)subMaskFrame->getDataPtr(), subMaskFrame->getStride());


        // Compute Upper and Lower Masks
        Mat upper_mask, lower_mask;
        computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask, mask); // This ones modifies inputImg

        // Sample Mask
        Mat upper_sampled_mask = upper_mask; //randomSampling<uchar>(upper_mask, 200, upper_mask);
        Mat lower_sampled_mask = lower_mask; //randomSampling<uchar>(lower_mask, 200, lower_mask);

        // Denoise Image
        //denoiseImage(copyImg, copyImg);

        // Convert from RGB color space to YCrCb
        Mat imgYuv;
        cv::cvtColor(inputImg, imgYuv, cv::COLOR_RGB2YCrCb);

        // Split image in Y, u and v channels
        vector<Mat> yuv_planes;
        split(imgYuv, yuv_planes);

        // Create 2D image with only u and v channels
        vector<Mat> uv_planes = {yuv_planes[1], yuv_planes[2]};
        Mat imgUv;
        merge(uv_planes, imgUv);

        // Compute the histogram for the upper (torso) and lower (leggs) parts
        auto u_hist = Histogram2D<uchar>::create(imgUv, {0, 255}, upper_sampled_mask);
        auto l_hist = Histogram2D<uchar>::create(imgUv, {0, 255}, lower_sampled_mask);

        const int n_often_colors = 8;

        // Show info of the histogram
        printHistogram<uchar, 2>(*u_hist, n_often_colors);

        // Create an image of the distribution of the histogram
        Mat histDist;
        createHistDistImage<uchar,2>({u_hist.get(), l_hist.get()},
                                     {Scalar(255, 0, 0), Scalar(0, 0, 255)}, // Blue (upper), Red (lower)
                                     histDist);

        // Create a 2D Image of the histogram
        float uv_range[] = {0.0f, 255.0f};
        Mat histImg;
        create2DCoordImage<uchar>({u_hist.get(), l_hist.get()},
                                  n_often_colors,
                                  {Vec3b(255, 0, 0), Vec3b(0, 0, 255)}, // Blue (upper), Red (lower)
                                  histImg,
                                  uv_range);

        // Modify input image to show used mask
        colorImageWithMask(inputImg, inputImg, upper_mask, lower_mask);

        // Show
        imshow("Hist.Dist", histDist);
        imshow("Hist.Img", histImg);
        waitKey(1);

    } // End OpenCV code

    return;
}

// Approach 4: RGB
// Paper: Color Invariants for Person Reidentification
void PrivacyFilter::approach4(QHashDataFrames &frames)
{
    Q_ASSERT(frames.contains(DataFrame::Color) && frames.contains(DataFrame::Mask) &&
             frames.contains(DataFrame::Skeleton) && frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(frames.value(DataFrame::Metadata));

    // Work in the Bounding Box
    if (metadataFrame->boundingBoxes().isEmpty())
        return;

    BoundingBox bb = metadataFrame->boundingBoxes().first();
    shared_ptr<ColorFrame> roiColorFrame = colorFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> roiMaskFrame = maskFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                             bb.size().width(), bb.size().height());

    // Start OpenCV code
    {using namespace cv;

        // Use cv::Mat for my color frame and mask frame
        Mat inputImg(roiColorFrame->getHeight(), roiColorFrame->getWidth(),
                     CV_8UC3, (void*)roiColorFrame->getDataPtr(), roiColorFrame->getStride());

        Mat mask(roiMaskFrame->getHeight(), roiMaskFrame->getWidth(),
                 CV_8UC1, (void*)roiMaskFrame->getDataPtr(), roiMaskFrame->getStride());

        // Denoise Image
        denoiseImage(inputImg, inputImg);

        // Discretise Image
        //discretiseRGBImage(inputImg, inputImg); // TEST: in YUV Space not RGB

        // Compute Upper and Lower Masks
        Mat upper_mask, lower_mask;
        computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask, mask);

        // Sample Mask
        Mat upper_sampled_mask = upper_mask; //randomSampling<uchar>(upper_mask, 1500, upper_mask);
        Mat lower_sampled_mask = lower_mask; //randomSampling<uchar>(lower_mask, 1500, lower_mask);

        // Convert to another color space
        Mat imgYuv;
        cv::cvtColor(inputImg, imgYuv, cv::COLOR_RGB2YCrCb); // YUV

        // Compute the histogram for the upper (torso) and lower (leggs) parts
        auto u_hist = Histogram3D<uchar>::create(imgYuv, {0, 255}, upper_sampled_mask);
        auto l_hist = Histogram3D<uchar>::create(imgYuv, {0, 255}, lower_sampled_mask);

        const int n_often_items = 72;

        // Create Distribution
        Mat distImg;
        createHistDistImage<uchar,3>({u_hist.get(), l_hist.get()},
                                     {Scalar(0, 255, 255), Scalar(0, 0, 255)}, // Blue (upper hist), Red (lower hist)
                                     distImg);

        // Create it on an image
        Mat colorPalette;
        create2DColorPalette<uchar>(u_hist->higherFreqBins(n_often_items), l_hist->higherFreqBins(n_often_items), colorPalette);
        cv::cvtColor(colorPalette, colorPalette, cv::COLOR_YCrCb2BGR); // YUV to BGR

        // Show
        imshow("Dist.Hist", distImg);
        imshow("Palette", colorPalette);
        waitKey(1);

    } // End OpenCV code
}

// Approach 5: RGB with buffering
// Paper: Color Invariants for Person Reidentification
void PrivacyFilter::approach5(QHashDataFrames& frames)
{
    Q_ASSERT(frames.contains(DataFrame::Color) && frames.contains(DataFrame::Mask) &&
             frames.contains(DataFrame::Skeleton) && frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(frames.value(DataFrame::Metadata));

    // Work in the Bounding Box
    if (metadataFrame->boundingBoxes().isEmpty())
        return;

    BoundingBox bb = metadataFrame->boundingBoxes().first();
    shared_ptr<ColorFrame> roiColorFrame = colorFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> roiMaskFrame = maskFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                             bb.size().width(), bb.size().height());

    static int frame_counter = 0;
    static const int buffer_size = 5;
    const int n_often_items = 72;
    static cv::Mat color_frame_vector[buffer_size];
    static cv::Mat mask_frame_vector[buffer_size];
    static bool finished = false;

    // Start OpenCV code
    {using namespace cv;

        // Use cv::Mat for my color frame and mask frame
        Mat inputImg(roiColorFrame->getHeight(), roiColorFrame->getWidth(),
                     CV_8UC3, (void*)roiColorFrame->getDataPtr(), roiColorFrame->getStride());

        Mat mask(roiMaskFrame->getHeight(), roiMaskFrame->getWidth(),
                 CV_8UC1, (void*)roiMaskFrame->getDataPtr(), roiMaskFrame->getStride());

        //
        // Image Pre-Processing
        //
        Mat img = inputImg.clone();

        // Denoise Image
        denoiseImage(img, img);

        // Discretise Image
        //discretiseRGBImage(img, img); // TEST: in YUV Space not RGB

        // Convert to another color space
        cv::cvtColor(img, img, cv::COLOR_RGB2YCrCb); // YUV

        //
        // Buffering
        //
        int current_idx = frame_counter % buffer_size;
        color_frame_vector[current_idx] = img;
        mask_frame_vector[current_idx] = mask.clone();

        // Start to do things when I fill up the buffer
        if (frame_counter >= buffer_size-1)
        {
            // Compute accumulated histogram of the current frame plus the previous fourth
            Histogram3c u_hist_acc, l_hist_acc;

            for (int i=0; i<buffer_size; ++i)
            {
                // Compute Upper and Lower Masks
                Mat upper_mask, lower_mask;
                computeUpperAndLowerMasks(color_frame_vector[i], upper_mask, lower_mask, mask_frame_vector[i]);

                // Sample Mask
                Mat upper_sampled_mask = upper_mask; //randomSampling<uchar>(upper_mask, 1500, upper_mask);
                Mat lower_sampled_mask = lower_mask; //randomSampling<uchar>(lower_mask, 1500, lower_mask);


                // Compute the histogram for the upper (torso) and lower (leggs) parts of each frame in the buffer
                auto u_hist_t = Histogram3D<uchar>::create(color_frame_vector[i], {0, 255}, upper_sampled_mask);
                auto l_hist_t = Histogram3D<uchar>::create(color_frame_vector[i], {0, 255}, lower_sampled_mask);

                // Accumulate it
                u_hist_acc += *u_hist_t;
                l_hist_acc += *l_hist_t;
            }

            // Show it on an image
            Mat colorPalette_acc;
            create2DColorPalette<uchar>(u_hist_acc.higherFreqBins(n_often_items), l_hist_acc.higherFreqBins(n_often_items), colorPalette_acc);
            cv::cvtColor(colorPalette_acc, colorPalette_acc, cv::COLOR_YCrCb2BGR); // YUV to BGR

            // Show Distribution
            Mat distImg;
            createHistDistImage<uchar,3>({&u_hist_acc, &l_hist_acc},
                                         {Scalar(255, 0, 0), Scalar(0, 0, 255)}, // Blue (upper hist), Red (lower hist)
                                         distImg);

            // Show
            imshow("Palette AC", colorPalette_acc);
            imshow("Hist.dist", distImg);
            waitKey(1);
        }
        else {
            qDebug() << "Buffering " << frame_counter % buffer_size;
        }

    } // End OpenCV code

    frame_counter++;
}

// Approach 6: Indexed colors
void PrivacyFilter::approach6(QHashDataFrames &frames)
{
    Q_ASSERT(frames.contains(DataFrame::Color) && frames.contains(DataFrame::Mask) &&
             frames.contains(DataFrame::Skeleton) && frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(frames.value(DataFrame::Metadata));

    // Work in the Bounding Box
    if (metadataFrame->boundingBoxes().isEmpty())
        return;

    BoundingBox bb = metadataFrame->boundingBoxes().first();
    shared_ptr<ColorFrame> roiColorFrame = colorFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> roiMaskFrame = maskFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                             bb.size().width(), bb.size().height());

    static int frame_counter = 0;
    static const int buffer_size = 5; // 190
    static cv::Mat color_frame_vector[buffer_size];
    static cv::Mat mask_frame_vector[buffer_size];

    // Start OpenCV code
    {using namespace cv;

        // Use cv::Mat for my color frame and mask frame
        Mat inputImg(roiColorFrame->getHeight(), roiColorFrame->getWidth(),
                     CV_8UC3, (void*)roiColorFrame->getDataPtr(), roiColorFrame->getStride());

        Mat mask(roiMaskFrame->getHeight(), roiMaskFrame->getWidth(),
                 CV_8UC1, (void*)roiMaskFrame->getDataPtr(), roiMaskFrame->getStride());

        // Color Palette with 8-8-4 levels
        Mat indexedImg = convertRGB2Indexed884(inputImg);

        //
        // Buffering
        //
        int current_idx = frame_counter % buffer_size;
        color_frame_vector[current_idx] = indexedImg;
        mask_frame_vector[current_idx] = mask.clone();

        // Start to do things when I fill up the buffer
        if (frame_counter >= buffer_size-1)
        {
            QList<shared_ptr<Histogram1c>> samples;
            Histogram1c u_hist_acc, l_hist_acc;

            for (int i=0; i<buffer_size; ++i)
            {
                // Compute Upper and Lower Masks
                Mat upper_mask, lower_mask;
                computeUpperAndLowerMasks(color_frame_vector[i], upper_mask, lower_mask, mask_frame_vector[i]);

                // Sample Mask
                Mat upper_sampled_mask = upper_mask; //randomSampling<uchar>(upper_mask, 1500, upper_mask);
                Mat lower_sampled_mask = lower_mask; //randomSampling<uchar>(lower_mask, 1500, lower_mask);

                // Compute the histogram for the upper (torso) and lower (leggs) parts of each frame in the buffer
                auto u_hist_t = Histogram1c::create(color_frame_vector[i], {0, 255}, upper_sampled_mask);
                auto l_hist_t = Histogram1c::create(color_frame_vector[i], {0, 255}, lower_sampled_mask);

                // Add to samples list
                samples << u_hist_t << l_hist_t;

                // Accumulate it
                u_hist_acc += *u_hist_t;
                l_hist_acc += *l_hist_t;
            }

            qDebug() << "u.items" << u_hist_acc.numItems() << "l.items" << l_hist_acc.numItems();
            qDebug() << "dist.ul" << Histogram1c::intersection(u_hist_acc, l_hist_acc);

            // KMeans
            qDebug() << "Computing K-Means...";
            const int k = 2;
            auto kmeans = KMeans<Histogram1c>::execute(samples, k);
            QList<Cluster<Histogram1c>> clusters = kmeans->getClusters();

            Mat hist_img[k];
            int i = 0;

            foreach (Cluster<Histogram1c> cluster, clusters)
            {
                if (!cluster.samples.isEmpty()) {
                    qDebug() << "Cluster" << i << "size" << cluster.samples.size();
                    shared_ptr<Histogram1c> hist = cluster.centroid;
                    createHistDistImage<uchar,1>({hist.get()}, {Scalar(0, 0, 255)}, hist_img[i]);
                    ++i;
                }
            }

            qDebug() << "Job Done";

            /*Mat hist_img[k];
            int img_idx = 0;

            foreach (shared_ptr<DistanceComparable> item, kmeans->getCentroids())
            {
                shared_ptr<Histogram3D<uchar>> hist = static_pointer_cast<Histogram3D<uchar>>(item);
                createHistImage(hist->sortedItems(n_often_items), hist_img[img_idx]);
                cv::cvtColor(hist_img[img_idx], hist_img[img_idx], cv::COLOR_YCrCb2BGR); // YUV to BGR
                img_idx++;
            }

            imshow("centroid1", hist_img[0]);
            imshow("centroid2", hist_img[1]);
            imshow("centroid3", hist_img[2]);
            imshow("centroid4", hist_img[3]);*/

            // Show it on an image
            /*Mat colorPalette_acc;
            create2DColorPalette<uchar>(u_hist_acc.sortedItems(n_often_items), l_hist_acc.sortedItems(n_often_items), colorPalette_acc);
            cv::cvtColor(colorPalette_acc, colorPalette_acc, cv::COLOR_YCrCb2BGR);*/ // YUV to BGR

            // Show Distribution
            /*Mat distImg;
            createHistDistImage<uchar,1>({&u_hist_acc, &l_hist_acc},
                                       0,
                                       {Scalar(0, 255, 255), Scalar(0, 0, 255)}, // Blue (upper hist), Red (lower hist)
                                       distImg);*/
            // Show
            //imshow("Palette AC", colorPalette_acc);

            //imshow("Hist.dist", distImg);
            imshow("hist1", hist_img[0]);
            imshow("hist2", hist_img[1]);
            waitKey(1);
        }
        else {
            qDebug() << "Buffering " << frame_counter % buffer_size;
        }
    } // End OpenCV code

    frame_counter++;
}

template <typename T, int N>
bool PrivacyFilter::compare(const cv::Mat& inputImg, const QList<Point<T,N>>& point_list, const cv::Mat& mask)
{
    Q_ASSERT( (mask.rows == 0 && mask.cols == 0) || (mask.rows == inputImg.rows && mask.cols == inputImg.cols) );

    auto it = point_list.constBegin();
    bool useMask = mask.rows > 0 && mask.cols > 0;
    bool equal = true;

    if (useMask) {
        if ( count_pixels_nz<uchar>(mask) != point_list.size())
            return false;
    } else {
        if (inputImg.rows * inputImg.cols != point_list.size())
            return false;
    }

    for (int i=0; i<inputImg.rows; ++i)
    {
        const cv::Vec<T,N>* pixel = inputImg.ptr<cv::Vec<T,N>>(i);
        const uchar* pMask = useMask ? mask.ptr<uchar>(i) : nullptr;

        for (int j=0; j<inputImg.cols; ++j)
        {
            if (useMask && pMask[j] <= 0)
                continue;

            const Point<T,N>& point = *it;
            int k = 0;

            while (k < N && equal)
            {
                equal = point[k] == pixel[j][k];
                ++k;
            }

            ++it;
            if (!equal) break;
        }

        if (!equal) break;
    }

    return equal;
}

template <class T, int N>
void PrivacyFilter::printHistogram(const Histogram<T, N> &hist, int n_elems) const
{
    // Show info of the histogram
    int i=0;
    foreach (auto item, hist.higherFreqBins(n_elems)) {
        qDebug() << i++ << "(" << item->point.toString() << ")" << item->value << item->dist;
    }
}

cv::Mat PrivacyFilter::createMask(cv::Mat input_img, int min_value, int* nonzero_counter, bool filter) const
{
    using namespace cv;

    Mat output_mask = Mat::zeros(input_img.rows, input_img.cols, CV_8UC1);
    int counter = 0;

    for (int i=0; i<input_img.rows; ++i)
    {
        float* pixel = input_img.ptr<float>(i);

        for (int j=0; j<input_img.cols; ++j)
        {
            if (pixel[j] > min_value) {
                output_mask.at<uchar>(i, j) = 1;
                counter++;
            } else if (filter){
                pixel[j] = 0;
            }
        }
    }

    if (nonzero_counter)
        *nonzero_counter = counter;

    return output_mask;
}

template <class T>
void PrivacyFilter::create2DColorPalette(const QList<const HistBin3D<T>*>& upper_hist, const QList<const HistBin3D<T>*>& lower_hist, cv::Mat& output_img) const
{
    using namespace cv;

    int width = 400;
    int height = 400;
    const int num_items = dai::max<int>(upper_hist.size(), lower_hist.size());

    int size = cvCeil(std::sqrt(num_items * 2));
    int size_offset = size % 2;

    const int num_rows = size + size_offset;
    const int num_cols = num_rows;
    const int total_cells = num_rows * num_cols;
    float color_height = float(height) / num_rows;
    float color_width = float(width) / num_cols;
    int row = 0, col = 0;

    output_img = Mat::zeros(height, width, CV_8UC3);

    for (int i=0; i<400; ++i)
    {
        Vec3b* pixel = output_img.ptr<Vec3b>(i);
        row = i / color_height;

        for (int j=0; j<400; ++j)
        {
            col = j / color_width;

            if (row < num_rows / 2) {
                int offset = row * num_rows + col;
                if (offset < upper_hist.size()) {
                    const HistBin3D<T>* item = upper_hist.at(offset);
                    pixel[j][0] = item->point[0]; // Y Blue
                    pixel[j][1] = item->point[1]; // u Green
                    pixel[j][2] = item->point[2]; // v Red
                }
            } else {
                int offset = (row * num_rows + col) - (total_cells / 2);
                if (offset < lower_hist.size()) {
                    const HistBin3D<T>* item = lower_hist.at(offset);
                    pixel[j][0] = item->point[0]; // Y Blue
                    pixel[j][1] = item->point[1]; // u Green
                    pixel[j][2] = item->point[2]; // v Red
                }
            }
        }
    }
}

template <class T>
void PrivacyFilter::createHistImage(const QList<const HistBin3D<T>*>& hist_items, cv::Mat& output_img) const
{
    using namespace cv;

    int width = 400;
    int height = 400;
    const int num_items = hist_items.size();
    int size = cvCeil(std::sqrt(num_items));
    int size_offset = size % 2;

    const int num_rows = size + size_offset;
    const int num_cols = num_rows;
    const int total_cells = num_rows * num_cols;
    float color_height = float(height) / num_rows;
    float color_width = float(width) / num_cols;
    int row = 0, col = 0;

    output_img = Mat::zeros(height, width, CV_8UC3);

    for (int i=0; i<400; ++i)
    {
        Vec3b* pixel = output_img.ptr<Vec3b>(i);
        row = i / color_height;

        for (int j=0; j<400; ++j)
        {
            col = j / color_width;
            int offset = row * num_rows + col;

            if (offset < num_items) {
                const HistBin3D<T>* item = hist_items.at(offset);
                pixel[j][0] = item->point[0]; // Y Blue
                pixel[j][1] = item->point[1]; // u Green
                pixel[j][2] = item->point[2]; // v Red
            }
        }
    }
}

template <class T>
void PrivacyFilter::create2DCoordImage(const QList<Histogram2D<T>*>& hist_list, int n_items, const QList<cv::Vec3b> &color_list, cv::Mat& output_img,
                                       float input_range[]) const
{
    Q_ASSERT(hist_list.size() == color_list.size());

    using namespace cv;

    const int width = 400;
    const int height = 400;

    output_img = Mat(height, width, CV_8UC3, Scalar(0,0,0));

    int color_idx = 0;

    for (auto it = hist_list.constBegin(); it != hist_list.constEnd(); ++it)
    {
        Vec3b color = color_list.at(color_idx);
        auto items = (*it)->higherFreqBins(n_items);

        foreach (auto item, items)
        {
            float coord_y = dai::normalise<T>(item->point[0], input_range[0], input_range[1], 0, height);
            float coord_x = dai::normalise<T>(item->point[1], input_range[0], input_range[1], 0, width);
            output_img.at<Vec3b>(coord_y, coord_x) = color;
        }

        ++color_idx;
    }
}

void PrivacyFilter::create2DCoordImage(const QList<QList<Point2f>*> &input_list, const QList<cv::Vec3b>& color_list,
                                       cv::Mat& output_img, float input_range[]) const
{
    Q_ASSERT(input_list.size() == color_list.size());

    using namespace cv;

    const int width = 500;
    const int height = 500;

    output_img = Mat(height, width, CV_8UC3, Scalar(0,0,0));

    int color_idx = 0;

    for (auto it = input_list.constBegin(); it != input_list.constEnd(); ++it)
    {
        Vec3b color = color_list.at(color_idx);
        QList<Point2f>* items = *it;

        foreach (auto point, *items)
        {
            float coord_x = dai::normalise<float>(point[0], input_range[0], input_range[1], 0, width);
            float coord_y = dai::normalise<float>(point[1], input_range[0], input_range[1], 0, height);
            output_img.at<Vec3b>(coord_y, coord_x) = color;
        }

        ++color_idx;
    }
}

template <class T>
void PrivacyFilter::create2DCoordImage(cv::Mat input_img, cv::Mat& output_img, int size[], float input_range[], bool init_output, cv::Vec3b color) const
{
    Q_ASSERT(input_img.channels() == 2);
    Q_ASSERT( init_output == true || (output_img.rows == size[0] && output_img.cols == size[1]) );

    using namespace cv;

    if (init_output)
        output_img = Mat::zeros(size[0], size[1], CV_8UC3);

    for (int i=0; i<input_img.rows; ++i)
    {
        T* pixel = input_img.ptr<T>(i);

        for (int j=0; j<input_img.cols; ++j)
        {
            float coord_y = dai::normalise<float>(pixel[j][0], input_range[0], input_range[1], 0, size[0]);
            float coord_x = dai::normalise<float>(pixel[j][1], input_range[0], input_range[1], 0, size[1]);
            output_img.at<Vec3b>(coord_y, coord_x) = color;
        }
    }
}

void PrivacyFilter::dilateUserMask(uint8_t *labels)
{
    int dilationSize = 18;
    cv::Mat newImag(480, 640, cv::DataType<uint8_t>::type, labels);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS,
                                               cv::Size(2*dilationSize + 1, 2*dilationSize+1),
                                               cv::Point( dilationSize, dilationSize ) );
    cv::dilate(newImag, newImag, kernel);
}

std::vector<cv::Rect> PrivacyFilter::faceDetection(cv::Mat frameGray, bool equalised)
{
    using namespace cv;
    std::vector<Rect> faces;

    if (!equalised) {
        equalizeHist( frameGray, frameGray );
    }

    // Detect faces
    //m_face_cascade.detectMultiScale( frameGray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30) );

    return faces;
}

} // End Namespace
