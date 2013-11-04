#ifndef VIEWERRENDERER_H
#define VIEWERRENDERER_H

#include <QtQuick/QQuickFramebufferObject>
#include "viewer/ViewerEngine.h"
#include "viewer/InstanceViewer.h"

class ViewerRenderer : public QQuickFramebufferObject::Renderer
{
public:
    ViewerRenderer(const InstanceViewer* viewer);
    void render();
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size);
    void setViewerEngine(ViewerEngine* engine);

protected:
    void synchronize(QQuickFramebufferObject *quickFbo);

private:
    ViewerEngine* m_viewerEngine;
    InstanceViewer* m_viewer;
};

#endif // VIEWERRENDERER_H
