#include "DepthFilter.h"
#include "types/ColorFrame.h"
#include "types/DepthFrame.h"
#include "types/SkeletonFrame.h"
#include <QDebug>

namespace dai {

DepthFilter::DepthFilter()
    : m_skelItem(nullptr)
    , m_bgItem(nullptr)
    , m_glContext(nullptr)
    , m_gles(nullptr)
    , m_initialised(false)
    //, m_scene(nullptr)
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

    m_skelItem = new SkeletonItem;
    m_skelItem->setMode3D(false);
    m_skelItem->initItem();

    m_bgItem = new BackgroundItem;
    m_bgItem->initItem();
    //m_scene = new Scene3DPainter;

    m_glContext->doneCurrent();
    m_initialised = true;
}

void DepthFilter::freeResources()
{
    if (m_glContext)
    {
        m_glContext->makeCurrent(&m_surface);

        if (m_bgItem) {
            delete m_bgItem;
            m_bgItem = nullptr;
        }

        if (m_skelItem) {
            delete m_skelItem;
            m_skelItem = nullptr;
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

shared_ptr<QHashDataFrames> DepthFilter::allocateMemory()
{
    m_frames = make_shared<QHashDataFrames>();
    return m_frames;
}

void DepthFilter::newFrames(const QHashDataFrames dataFrames)
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

void DepthFilter::produceFrames(QHashDataFrames& output)
{
    bool background = false;

    // Convert Depth Frame to Color
    if (output.contains(DataFrame::Depth))
    {
        shared_ptr<DepthFrame> depthFrame = static_pointer_cast<DepthFrame>(output.value(DataFrame::Depth));
        shared_ptr<ColorFrame> colorFrame;

        if (output.contains(DataFrame::Color)) {
            colorFrame = static_pointer_cast<ColorFrame>(output.value(DataFrame::Color));
        } else {
            colorFrame = make_shared<ColorFrame>(depthFrame->width(), depthFrame->height());
            output.insert(DataFrame::Color, colorFrame);
        }

        QMap<uint16_t, float> colorHistogram;
        DepthFrame::calculateHistogram(colorHistogram, *(depthFrame.get()));

        for (int i=0; i<depthFrame->height(); ++i)
        {
            const uint16_t* pDepth = depthFrame->getRowPtr(i);
            RGBColor* pColor = colorFrame->getRowPtr(i);

            for (int j=0; j<depthFrame->width(); ++j)
            {
               uint8_t color = colorHistogram[pDepth[j]];
               pColor[j].red = color;
               pColor[j].green = color;
               pColor[j].blue = 0;
            }
        }

        background = true;
    }

    // Render Skeleton
    if (output.contains(DataFrame::Skeleton))
    {
        shared_ptr<SkeletonFrame> skelFrame = static_pointer_cast<SkeletonFrame>(output.value(DataFrame::Skeleton));
        shared_ptr<ColorFrame> colorFrame;

        if (output.contains(DataFrame::Color)) {
            colorFrame = static_pointer_cast<ColorFrame>(output.value(DataFrame::Color));
        } else {
            colorFrame = make_shared<ColorFrame>(640, 480);
            output.insert(DataFrame::Color, colorFrame);
        }

        // Render
        m_glContext->makeCurrent(&m_surface);
        m_fboDisplay->bind();

        // Clear Screen
        m_gles->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        m_gles->glClearDepthf(1.0f);
        m_gles->glViewport(0, 0, 640, 480);
        m_gles->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (background) {
            // Render Background
            m_bgItem->setBackground(colorFrame);
            m_bgItem->renderItem();
        }

        // Render Item
        m_skelItem->setSkeleton(skelFrame);
        m_skelItem->renderItem();

        // Copy data back to ColorFrame
        m_gles->glFlush();
        m_gles->glReadPixels(0,0, 640, 480, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) colorFrame->getDataPtr());
        m_fboDisplay->release();
        m_glContext->doneCurrent();
    }
}

// Render a 3D Scene (old version)
/*QHashDataFrames DepthFilter::produceFrames()
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
}*/

} // End Namespace
