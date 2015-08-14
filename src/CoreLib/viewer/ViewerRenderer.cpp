#include "ViewerRenderer.h"
#include <QOpenGLFramebufferObject>
#include "viewer/ViewerEngine.h"
#include "viewer/InstanceViewer.h"

ViewerRenderer::ViewerRenderer()
    : m_viewerEngine(nullptr)
{
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
}
