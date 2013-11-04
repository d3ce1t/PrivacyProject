#include "ViewerRenderer.h"
#include <QOpenGLFramebufferObject>

ViewerRenderer::ViewerRenderer(const InstanceViewer *viewer)
    : m_viewerEngine(nullptr)
{
    m_viewer = const_cast<InstanceViewer*>(viewer);
}

void ViewerRenderer::setViewerEngine(ViewerEngine* engine)
{
    Q_ASSERT(engine != nullptr);
    m_viewerEngine = engine;
}

void ViewerRenderer::render()
{
    if (m_viewerEngine) {
        m_viewerEngine->renderOpenGLScene(this->framebufferObject());
    }

    update();
}

QOpenGLFramebufferObject* ViewerRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    return new QOpenGLFramebufferObject(size, format);
}

void ViewerRenderer::synchronize(QQuickFramebufferObject *quickFbo)
{
    if (m_viewerEngine)
        m_viewerEngine->setSize(QSize(quickFbo->width(), quickFbo->height()));
}
