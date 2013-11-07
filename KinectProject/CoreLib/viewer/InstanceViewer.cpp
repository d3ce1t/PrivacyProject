#include "InstanceViewer.h"
#include <QDebug>
#include <iostream>
#include "viewer/ViewerRenderer.h"
#include <QThread>

InstanceViewer::InstanceViewer()
    : QQuickFramebufferObject()
    , m_viewerEngine(nullptr)
{
    setTextureFollowsItemSize(false);
    setSmooth(false);
    startTimer(60);
}

InstanceViewer::~InstanceViewer()
{
    qDebug() << "InstanceViewer::~InstanceViewer()";
}

void InstanceViewer::setViewerEngine(ViewerEngine *viewerEngine)
{
    Q_ASSERT(viewerEngine != nullptr);
    qDebug() << QThread::currentThreadId();
    m_viewerEngine = viewerEngine;
}

QQuickFramebufferObject::Renderer* InstanceViewer::createRenderer() const
{
    qDebug() << QThread::currentThreadId();
    ViewerRenderer* renderer = new ViewerRenderer(this);
    renderer->setViewerEngine(m_viewerEngine);
    return renderer;
}

void InstanceViewer::timerEvent(QTimerEvent *evt)
{
    Q_UNUSED(evt);
    update();
}
