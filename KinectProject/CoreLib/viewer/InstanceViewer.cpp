#include "InstanceViewer.h"
#include <QDebug>
#include <iostream>
#include "viewer/ViewerRenderer.h"
#include <QThread>

InstanceViewer::InstanceViewer()
    : QQuickFramebufferObject()
    , m_viewerEngine(nullptr)
{
    setFlag(QQuickItem::ItemHasContents);
    setTextureFollowsItemSize(false);
    setSmooth(false);
    m_timer = startTimer(35);
}

InstanceViewer::~InstanceViewer()
{
    //killTimer(m_timer);
    qDebug() << "InstanceViewer::~InstanceViewer()";
}

void InstanceViewer::setViewerEngine(ViewerEngine *viewerEngine)
{
    Q_ASSERT(viewerEngine != nullptr);
    viewerEngine->setInstanceViewer(this);
    m_viewerEngine = viewerEngine;
}

QQuickFramebufferObject::Renderer* InstanceViewer::createRenderer() const
{
    ViewerRenderer* renderer = new ViewerRenderer;
    renderer->setViewerEngine(m_viewerEngine);
    return renderer;
}

void InstanceViewer::timerEvent(QTimerEvent *evt)
{
    Q_UNUSED(evt);
    update();
}
