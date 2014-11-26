#ifndef VIEWERRENDERER_H
#define VIEWERRENDERER_H

#include <QtQuick/QQuickFramebufferObject>

class ViewerEngine;

class ViewerRenderer : public QQuickFramebufferObject::Renderer
{
public:
    ViewerRenderer();
    void setViewerEngine(ViewerEngine* engine);

protected:
    void render() override;

private:
    ViewerEngine* m_viewerEngine;
};

#endif // VIEWERRENDERER_H
