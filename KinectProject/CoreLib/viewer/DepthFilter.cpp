#include "DepthFilter.h"
#include "viewer/SkeletonItem.h"
#include "types/ColorFrame.h"

namespace dai {

DepthFilter::DepthFilter()
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

DepthFilter::~DepthFilter()
{
    stopListener();
    freeResources();
    qDebug() << "DepthFilter::~DepthFilter";
}

void DepthFilter::initialise()
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

    m_scene = new Scene3DPainter;

    m_glContext->doneCurrent();
    m_initialised = true;
}

void DepthFilter::freeResources()
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

void DepthFilter::newFrames(const QHashDataFrames dataFrames)
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
        if (subscribersCount() == 0 || !generate()) {
            qDebug() << "DepthFilter: No listeners or Nothing produced";
            stopListener();
        }
    }
    else {
        qDebug() << "Frame has expired!";
    }
}

void DepthFilter::afterStop()
{
    freeResources();
}

QHashDataFrames DepthFilter::produceFrames()
{
    shared_ptr<SkeletonItem> skeletonItem = static_pointer_cast<SkeletonItem>(m_scene->getFirstItem(ITEM_SKELETON));

    // Clear items of the scene
    m_scene->clearItems();

    // Background
    m_scene->setBackground( m_frames.value(DataFrame::Depth) );

    // Add skeleton item to the scene
    if (m_frames.contains(DataFrame::Skeleton))
    {
        if (!skeletonItem)
            skeletonItem = make_shared<SkeletonItem>();

        skeletonItem->setSkeleton( static_pointer_cast<SkeletonFrame>(m_frames.value(DataFrame::Skeleton)) );
        m_scene->addItem(skeletonItem);
    }

    m_scene->markAsDirty();

    //
    // Render scene offscreen
    //
    shared_ptr<ColorFrame> colorFrame;

    if (m_frames.contains(DataFrame::Color)) {
        colorFrame = static_pointer_cast<ColorFrame>(m_frames.value(DataFrame::Color));
    } else {
        colorFrame = make_shared<ColorFrame>(640, 480);
        m_frames.insert(DataFrame::Color, colorFrame);
    }

    m_glContext->makeCurrent(&m_surface);
    m_scene->setSize(m_fboDisplay->width(), m_fboDisplay->height());
    m_scene->renderScene(m_fboDisplay);

    m_fboDisplay->bind();
    m_gles->glFlush();

    // Copy data back to ColorFrame
    m_gles->glReadPixels(0,0, 640, 480, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) colorFrame->getDataPtr());
    m_fboDisplay->release();
    m_glContext->doneCurrent();

    return m_frames;
}

} // End Namespace
