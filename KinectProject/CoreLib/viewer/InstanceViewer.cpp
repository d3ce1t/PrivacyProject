#include "InstanceViewer.h"
#include <QDebug>
#include <QQmlContext>
#include <QtQml>
#include <QQuickWindow>
#include <iostream>

InstanceViewer::InstanceViewer()
    : m_viewerEngine(nullptr)
{
    // QML Setup
    connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(handleWindowChanged(QQuickWindow*)));   
}

InstanceViewer::~InstanceViewer()
{
    qDebug() << "InstanceViewer::~InstanceViewer()";
}

void InstanceViewer::handleWindowChanged(QQuickWindow *win)
{
    if (win) {
        connect(win, &QQuickWindow::beforeRendering, this, &InstanceViewer::renderOpenGLScene, Qt::DirectConnection);
        //connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(sync()), Qt::DirectConnection);
        //connect(win->openglContext(), SIGNAL(aboutToBeDestroyed()), this, SLOT(cleanup()), Qt::DirectConnection);
        win->setClearBeforeRendering(false);
        m_quickWindow = win;
    }
}

void InstanceViewer::setViewerEngine(ViewerEngine *viewerEngine)
{
    Q_ASSERT(viewerEngine != nullptr);

    m_viewerEngine = viewerEngine;
}

void InstanceViewer::renderOpenGLScene()
{
    if (m_viewerEngine != nullptr)
        m_viewerEngine->renderOpenGLScene();
}

