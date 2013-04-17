#include "InstanceViewer.h"
#include "dataset/DataInstance.h"
#include "types/DepthFrame.h"
#include "types/Skeleton.h"
#include "ViewerPainter.h"
#include <QDebug>
#include <QQmlContext>
#include <QtQml>
#include <iostream>
#include <Grill.h>
#include <HistogramScene.h>
#include "dataset/InstanceInfo.h"


InstanceViewer::InstanceViewer( QWindow *parent )
    : QQuickView( parent )
{
    // QML Setup
    rootContext()->setContextProperty("winObject", (QObject *) this);
    QQuickView::setResizeMode( QQuickView::SizeRootObjectToView);
    QQuickView::setSource(QUrl("qrc:///scenegraph/openglunderqml/main.qml"));
    setClearBeforeRendering( false );
    QObject::connect(this, SIGNAL(beforeRendering()), SLOT(renderOpenGLScene()), Qt::DirectConnection);
    QObject::connect(this, SIGNAL(frameSwapped()), SLOT(renderLater()), Qt::DirectConnection);

    // Viewer Setup
    this->setTitle("Instance Viewer");
    this->stop();
    resetPerspective();
}

InstanceViewer::~InstanceViewer()
{
    stop();

    m_mutex.lock();
    foreach (dai::ViewerPainter* painter, m_painters) {
        delete &(painter->instance());
        delete painter;
    }
    m_painters.clear();
    m_mutex.unlock();
}

void InstanceViewer::show() {
    QQuickView::show();
}

void InstanceViewer::play(dai::DataInstance* instance, bool restartAll)
{
    this->setTitle("Instance Viewer (" + instance->getMetadata().getFileName() + ")");
    dai::InstanceInfo::InstanceType instanceType = instance->getMetadata().getType();
    dai::ViewerPainter* painter;

    if (instanceType == dai::InstanceInfo::Depth) {
        painter = new dai::DepthFramePainter(instance, this);
    }
    else if (instanceType == dai::InstanceInfo::Skeleton) {
        painter = new dai::SkeletonPainter(instance, this);
    }

    m_mutex.lock();
    m_painters.append(painter);
    m_mutex.unlock();

    updatePaintersMatrix();
    instance->open();

    if (restartAll) {
        m_mutex.lock();
        QListIterator<dai::ViewerPainter*> it(m_painters);
        while (it.hasNext()) {
            dai::ViewerPainter* painter = it.next();
            dai::DataInstance& instance = painter->instance();
            instance.close();
            instance.open();
        }
        m_mutex.unlock();
    }

    m_frames = 0;
    m_lastTime = 0;
    m_update_pending = false;
    m_running = true;
    m_time.start();
    renderLater();
    qDebug() << "Playing";
}

void InstanceViewer::stop()
{
    m_mutex.lock();
    foreach (dai::ViewerPainter* painter, m_painters)
    {
        dai::DataInstance& instance = painter->instance();
        instance.close();
    }
    m_mutex.unlock();

    m_frames = 0;
    m_fps = 0;
    m_lastTime = 0;
    m_running = false;
    m_update_pending = false;
    emit changeOfStatus();
    qDebug() << "Stopped";
}

void InstanceViewer::playNextFrame()
{
    // Compute time since last update
    const qint64 sleepTime = 70;
    qint64 timeNow = m_time.elapsed();
    qint64 diffTime = timeNow - m_lastTime;

    if (m_running && diffTime >= sleepTime)
    {
        // Compute Frame Per Seconds
        m_frames++;
        m_fps = 1.0 / (diffTime / 1000.0f);
        m_lastTime = timeNow;

        // Do Job
        m_mutex.lock();
        QListIterator<dai::ViewerPainter*> it(m_painters);
        QList<dai::ViewerPainter*> closedPainters;

        while (it.hasNext()) {
            dai::ViewerPainter* painter = it.next();
            bool result = painter->prepareNext();

            // Instance associated with this painter is closed or has failed. So I don't
            // use this painter anymore.
            if (result == false) {
                closedPainters.append(painter);
            }
        }

        // Remove closed painters
        if (!closedPainters.isEmpty()) {
            QListIterator<dai::ViewerPainter*> it(closedPainters);
            while (it.hasNext()) {
                dai::ViewerPainter* painter = it.next();
                m_painters.removeAll(painter);
            }
            closedPainters.clear();
        }

        // Update Viewer
        if (!m_painters.isEmpty()) {
            emit changeOfStatus();
            QQuickView::update();
        } else {
            stop();
        }

         m_mutex.unlock();
    }
    else if (m_running) {
         QTimer::singleShot(sleepTime - diffTime, this, SLOT(playNextFrame()));
    }
}

void InstanceViewer::renderOpenGLScene()
{
    // Init Each Frame (because QtQuick could change it)
    glDepthRange(0.0f, 1.0f);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    // Configure ViewPort and Clear Screen
    glViewport(0, 0, width(), height());
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Draw
    if (m_update_pending) {
        m_mutex.lock();
        QListIterator<dai::ViewerPainter*> it(m_painters);

        while (it.hasNext()) {
            dai::ViewerPainter* painter = it.next();
            painter->renderNow();
        }
        m_mutex.unlock();
    }

    // Restore
    glDisable(GL_DEPTH_TEST);
    m_update_pending = false;
}

void InstanceViewer::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
}

void InstanceViewer::updatePaintersMatrix()
{
    m_mutex.lock();
    QListIterator<dai::ViewerPainter*> it(m_painters);

    while (it.hasNext()) {
        dai::ViewerPainter* painter = it.next();
        painter->setMatrix(matrix);
    }
    m_mutex.unlock();
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

void InstanceViewer::renderLater()
{
    if (m_running && !m_update_pending) {
        m_update_pending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

bool InstanceViewer::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        playNextFrame();
        return true;
    case QEvent::Close:
        // PATCH
        emit viewerClose(this);
        return true;
        break;
    default:
        return QQuickView::event(event);
    }
}
