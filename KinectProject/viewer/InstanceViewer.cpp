#include "InstanceViewer.h"
#include "../dataset/DataInstance.h"
#include <QDebug>
#include <QQmlContext>
#include <QtQml>
#include <iostream>
#include "../Grill.h"
#include "../HistogramScene.h"
#include "../dataset/InstanceInfo.h"
#include "DepthFramePainter.h"
#include "SkeletonPainter.h"
#include "ColorFramePainter.h"

InstanceViewer::InstanceViewer( QWindow *parent )
    : QQuickView( parent )
{
    // QML Setup
    rootContext()->setContextProperty("winObject", (QObject *) this);
    //QQuickView::setResizeMode( QQuickView::SizeRootObjectToView);
    QQuickView::setSource(QUrl("qrc:///qml/qml/main.qml"));
    setClearBeforeRendering( false );
    QObject::connect(this, SIGNAL(beforeRendering()), SLOT(renderOpenGLScene()), Qt::DirectConnection);

    // Viewer Setup
    this->setTitle("Instance Viewer");
    m_running = false;
    m_playback = NULL;
    m_fps = 0.0;
    m_token = -1;
    resetPerspective();
    //initializeOpenGLFunctions();
}

InstanceViewer::~InstanceViewer()
{
    stop();

    m_mutex.lock();
    foreach (dai::Painter* painter, m_painters) {
        //delete &(painter->instance());
        delete painter;
    }
    m_painters.clear();
    m_mutex.unlock();
}

void InstanceViewer::show() {
    QQuickView::show();
}

void InstanceViewer::addInstance(dai::StreamInstance* instance)
{
    // this->setTitle("Instance Viewer (" + instance->getTitle() + ")");
    dai::StreamInstance::StreamType streamType = instance->getType();
    dai::Painter* painter;

    if (streamType == dai::StreamInstance::Depth) {
        painter = new dai::DepthFramePainter(instance, this);
    }
    else if (streamType == dai::StreamInstance::Skeleton) {
        painter = new dai::SkeletonPainter(instance, this);
    }
    else if (streamType == dai::StreamInstance::Color) {
        painter = new dai::ColorFramePainter(instance, this);
    }

    m_mutex.lock();
    m_painters.append(painter);
    m_mutex.unlock();

    updatePaintersMatrix();
}

void InstanceViewer::play(dai::StreamInstance *instance, bool restartAll)
{
  /*
    instance->open();

    if (restartAll) {
        m_mutex.lock();
        QListIterator<dai::Painter*> it(m_painters);
        while (it.hasNext()) {
            dai::Painter* painter = it.next();
            dai::StreamInstance& instance = painter->instance();
            instance.close();
            instance.open();
        }
        m_mutex.unlock();
    }

    m_update_pending = false;
    m_running = true;
    renderLater();
    qDebug() << "Playing";*/
}

void InstanceViewer::stop()
{
    /*m_mutex.lock();
    foreach (dai::Painter* painter, m_painters)
    {
        dai::StreamInstance& instance = painter->instance();
        instance.close();
    }
    m_mutex.unlock();
    m_running = false;
    m_update_pending = false;
    emit changeOfStatus();
    qDebug() << "Stopped";*/
}

void InstanceViewer::setPlayback(dai::PlaybackControl* playback)
{
    m_playback = playback;
    connect(playback, SIGNAL(newFrameRead()), this, SLOT(playNextFrame()));
}

void InstanceViewer::playNextFrame()
{
    m_token = m_playback->acquire();
    m_running = true;
    m_fps = m_playback->getFPS();
    //emit beforeDisplaying(framesList, this);
    QQuickView::update();
}

void InstanceViewer::renderOpenGLScene()
{
    // Init Each Frame (because QtQuick could change it)
    glDepthRangef(0.0f, 1.0f);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    // Configure ViewPort and Clear Screen
    glViewport(0, 0, width(), height());
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Draw
    if (m_running) {
        m_mutex.lock();
        QListIterator<dai::Painter*> it(m_painters);

        while (it.hasNext()) {
            dai::Painter* painter = it.next();
            painter->renderNow();
        }
        m_mutex.unlock();

        emit changeOfStatus();
    }

    m_playback->release(m_token);
    m_token = -1;

    // Restore
    glDisable(GL_DEPTH_TEST);
}

void InstanceViewer::updatePaintersMatrix()
{
    m_mutex.lock();
    QListIterator<dai::Painter*> it(m_painters);

    while (it.hasNext()) {
        dai::Painter* painter = it.next();
        painter->setMatrix(matrix);
    }
    m_mutex.unlock();
    QQuickView::update();
}

void InstanceViewer::resetPerspective()
{
    matrix.setToIdentity();
    matrix.perspective(70, 1.0, 0.01, 10.0);
    matrix.translate(0, 0, -1.5);
    updatePaintersMatrix();
}

void InstanceViewer::rotateAxisX(float angle)
{
    matrix.rotate(angle, QVector3D(1, 0, 0));
    updatePaintersMatrix();
}

void InstanceViewer::rotateAxisY(float angle)
{
    matrix.rotate(angle, QVector3D(0, 1, 0));
    updatePaintersMatrix();
}

void InstanceViewer::rotateAxisZ(float angle)
{
    matrix.rotate(angle, QVector3D(0, 0, 1));
    updatePaintersMatrix();
}

void InstanceViewer::translateAxisX(float value)
{
    matrix.translate(value, 0, 0);
    updatePaintersMatrix();
}

void InstanceViewer::translateAxisY(float value)
{
    matrix.translate(0, value, 0);
    updatePaintersMatrix();
}

void InstanceViewer::translateAxisZ(float value)
{
    matrix.translate(0, 0, value);
    updatePaintersMatrix();
}

float InstanceViewer::getFPS() const
{
    return m_fps;
}

bool InstanceViewer::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::Close:
        // PATCH
        emit viewerClose(this);
        return true;
        break;
    default:
        return QQuickView::event(event);
    }
}
