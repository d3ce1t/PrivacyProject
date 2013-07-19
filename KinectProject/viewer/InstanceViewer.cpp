#include "InstanceViewer.h"
#include <QDebug>
#include <QQmlContext>
#include <QtQml>
#include <QQuickWindow>
#include <iostream>
#include "DepthFramePainter.h"
#include "SkeletonPainter.h"
#include "ColorFramePainter.h"
#include "UserFramePainter.h"

InstanceViewer::InstanceViewer()
{
    // QML Setup
    connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(handleWindowChanged(QQuickWindow*)));

    // Viewer Setup
    m_painters.insert(dai::DataFrame::Color, new dai::ColorFramePainter(nullptr));
    m_painters.insert(dai::DataFrame::Depth, new dai::DepthFramePainter(nullptr));
    m_painters.insert(dai::DataFrame::Skeleton, new dai::SkeletonPainter(nullptr));
    m_painters.insert(dai::DataFrame::User, new dai::UserFramePainter(nullptr));

    m_running = false;
    m_window = nullptr;
    resetPerspective();
}

InstanceViewer::~InstanceViewer()
{
    m_mutex.lock();
    foreach (dai::Painter* painter, m_painters.values()) {
        delete painter;
    }
    m_painters.clear();
    m_mutex.unlock();
    m_running = false;
    m_window = nullptr;
    qDebug() << "InstanceViewer::~InstanceViewer()";
}

void InstanceViewer::onNewFrame(QList<shared_ptr<dai::DataFrame> > dataFrames)
{
    m_running = true;

    shared_ptr<dai::UserFrame> userMask;
    int i = 0;

    // Get UserFrame
    while (!userMask && i < dataFrames.size())
    {
        shared_ptr<dai::DataFrame> frame = dataFrames.at(i);

        if (frame->getType() == dai::DataFrame::User) {
            userMask = static_pointer_cast<dai::UserFrame>(frame);
            if (dataFrames.size() > 1) // I only show user mask if it's the only one frame
                dataFrames.removeAt(i);
        }

        i++;
    }

    m_mutex.lock();
    foreach (shared_ptr<dai::DataFrame> frame, dataFrames) {
        dai::Painter* painter = m_painters.value(frame->getType());
        if (painter) {
            painter->setMask(userMask);
            painter->prepareData(frame);
            painter->setMask(nullptr);
        }
    }
    m_mutex.unlock();

    if (m_window != nullptr)
        m_window->update();
}

void InstanceViewer::renderOpenGLScene()
{
    // Init Each Frame (because QtQuick could change it)
    glDepthRange(0.0f, 1.0f);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Configure ViewPort and Clear Screen
    glViewport(0, 0, width(), height());

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Draw
    if (m_running)
    {
        m_mutex.lock();
        foreach (dai::Painter* painter, m_painters.values()) {
            painter->renderNow();
        }
        m_mutex.unlock();
        emit frameRendered();
    }

    // Restore
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
}

void InstanceViewer::handleWindowChanged(QQuickWindow *win)
{
    if (win) {
        connect(win, SIGNAL(beforeRendering()), this, SLOT(renderOpenGLScene()), Qt::DirectConnection);
        //connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(sync()), Qt::DirectConnection);
        //connect(win->openglContext(), SIGNAL(aboutToBeDestroyed()), this, SLOT(cleanup()), Qt::DirectConnection);
        win->setClearBeforeRendering(false);
        m_window = win;
    }
}

/*void InstanceViewer::sync()
{
}*/

void InstanceViewer::updatePaintersMatrix()
{
    m_mutex.lock();
    foreach (dai::Painter* painter, m_painters.values()) {
        painter->setMatrix(m_matrix);
    }
    m_mutex.unlock();

    if (m_window != nullptr)
        m_window->update();
}

void InstanceViewer::resetPerspective()
{
    m_matrix.setToIdentity();
    m_matrix.perspective(70, 1.0, 0.01, 10.0);
    m_matrix.translate(0, 0, -1.425);
    updatePaintersMatrix();
}

void InstanceViewer::rotateAxisX(float angle)
{
    m_matrix.rotate(angle, QVector3D(1, 0, 0));
    updatePaintersMatrix();
}

void InstanceViewer::rotateAxisY(float angle)
{
    m_matrix.rotate(angle, QVector3D(0, 1, 0));
    updatePaintersMatrix();
}

void InstanceViewer::rotateAxisZ(float angle)
{
    m_matrix.rotate(angle, QVector3D(0, 0, 1));
    updatePaintersMatrix();
}

void InstanceViewer::translateAxisX(float value)
{
    m_matrix.translate(value, 0, 0);
    updatePaintersMatrix();
}

void InstanceViewer::translateAxisY(float value)
{
    m_matrix.translate(0, value, 0);
    updatePaintersMatrix();
}

void InstanceViewer::translateAxisZ(float value)
{
    m_matrix.translate(0, 0, value);
    updatePaintersMatrix();
}
