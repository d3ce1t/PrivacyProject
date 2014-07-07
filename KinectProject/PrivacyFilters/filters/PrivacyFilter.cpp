#include "PrivacyFilter.h"
#include "types/MaskFrame.h"
#include <opencv2/opencv.hpp>
#include "viewer/SilhouetteItem.h"
#include "viewer/SkeletonItem.h"
#include <QImage>
#include <QColor>
#include <QDebug>

namespace dai {

PrivacyFilter::PrivacyFilter()
    : m_glContext(nullptr)
    , m_gles(nullptr)
    , m_initialised(false)
    , m_scene(nullptr)
    , m_fboDisplay(nullptr)
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
}

PrivacyFilter::~PrivacyFilter()
{
   stopListener();
   freeResources();
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

    m_scene = new Scene2DPainter;

    m_glContext->doneCurrent();
    m_initialised = true;
}

void PrivacyFilter::freeResources()
{
    if (m_glContext)
    {
        m_glContext->makeCurrent(&m_surface);

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

// This method is called from a thread from the PlaybackControl
void PrivacyFilter::newFrames(const QHashDataFrames dataFrames)
{
    if (!m_initialised) {
        initialise();
    }

    // Copy frames (1 ms)
    m_frames.clear();

    foreach (DataFrame::FrameType key, dataFrames.keys()) {
        shared_ptr<DataFrame> frame = dataFrames.value(key);
        m_frames.insert(key, frame->clone());
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
        qDebug() << "Frame has expired!";
    }
}

void PrivacyFilter::afterStop()
{
    freeResources();
}

QHashDataFrames PrivacyFilter::produceFrames()
{
    //
    // Dilate mask to create a wide border (value = 255)
    //
    if (m_frames.contains(DataFrame::Mask))
    {
         shared_ptr<MaskFrame> inputMask = static_pointer_cast<MaskFrame>(m_frames.value(DataFrame::Mask));
         shared_ptr<MaskFrame> outputMask = static_pointer_cast<MaskFrame>(inputMask->clone());
         dilateUserMask(const_cast<uint8_t*>(outputMask->getDataPtr()));

         for (int i=0; i<inputMask->getHeight(); ++i)
         {
             for (int j=0; j<inputMask->getWidth(); ++j)
             {
                 uint8_t inputValue = inputMask->getItem(i,j);
                 uint8_t outputValue = outputMask->getItem(i,j);

                 if (inputValue == 0 && outputValue > 0) {
                     inputMask->setItem(i, j, uint8_t(255));
                 }
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
    if (m_frames.contains(DataFrame::Color))
    {
        if (m_frames.contains(DataFrame::Mask))
        {
            m_scene->setMask(static_pointer_cast<MaskFrame>(m_frames.value(DataFrame::Mask)));

            // Add silhuette item to the scene
            if (!silhouetteItem)
                silhouetteItem.reset(new SilhouetteItem);

            //silhouetteItem->setDrawingEffect(SilhouetteItem::EFFECT_BLUR);
            silhouetteItem->setUser( static_pointer_cast<MaskFrame>(m_frames.value(DataFrame::Mask)) );
            m_scene->addItem(silhouetteItem);
        }

        m_scene->setBackground(m_frames.value(DataFrame::Color));
    }

    // Add skeleton item to the scene
    if (m_frames.contains(DataFrame::Skeleton))
    {
        if (!skeletonItem)
            skeletonItem.reset(new SkeletonItem);

        skeletonItem->setSkeleton( static_pointer_cast<SkeletonFrame>(m_frames.value(DataFrame::Skeleton)) );
        m_scene->addItem(skeletonItem);
    }

    m_scene->enableFilter(QMLEnumsWrapper::FILTER_BLUR);
    m_scene->markAsDirty();

    //
    // Render scene offscreen
    //
    if (m_frames.contains(DataFrame::Color))
    {
        shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(m_frames.value(DataFrame::Color));
        m_glContext->makeCurrent(&m_surface);
        m_scene->setSize(m_fboDisplay->width(), m_fboDisplay->height());
        m_scene->renderScene(m_fboDisplay);

        // We need to flush the contents to the FBO before posting
        // the texture to the other thread, otherwise, we might
        // get unexpected results.
        m_gles->glFlush();

        // Copy data back to ColorFrame
        m_gles->glReadPixels(0,0, 640, 480, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) colorFrame->getDataPtr());
        m_glContext->doneCurrent();
    }

    return m_frames;
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

/*void PrivacyFilter::blurEffect(shared_ptr<DataFrame> frame)
{
    ColorFrame* colorFrame = (ColorFrame*) frame.get();
    ColorFrame  background = *colorFrame;
    cv::Mat newImag(480, 640, CV_8UC3,  const_cast<RGBColor*>(colorFrame->getDataPtr()));

    cv::GaussianBlur( newImag, newImag, cv::Size( 29, 29 ), 0, 0 );

    for (int i=0; i<480; ++i) {
        for (int j=0; j<640; ++j) {
            if (m_userMask->getItem(i, j) == 0) {
                RGBColor color = background.getItem(i, j);
                colorFrame->setItem(i, j, color);
            }
        }
    }
}

void PrivacyFilter::invisibilityEffect(shared_ptr<DataFrame> frame)
{
    if (m_userMask == nullptr)
        return;

    ColorFrame* colorFrame = (ColorFrame*) frame.get();

    // Get initial background at 20th frame
    if (colorFrame->getIndex() == 20) {
        m_background = *colorFrame;
    }

    // Update Background and make replacement
    for (int i=0; i<m_userMask->getHeight(); ++i)
    {
        for (int j=0; j<m_userMask->getWidth(); ++j)
        {
            uint8_t uLabel = m_userMask->getItem(i, j);

            if (uLabel != 0) {
                if (m_enabled) {
                    RGBColor bgColor = m_background.getItem(i, j);
                    colorFrame->setItem(i, j, bgColor);
                }
            }
            else {
                RGBColor color = colorFrame->getItem(i, j);
                m_background.setItem(i, j, color);
            }
        }
    }
}*/

} // End Namespace
