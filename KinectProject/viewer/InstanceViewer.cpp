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
    : QQuickView( parent )//, g_poseTimeoutToExit(2000)
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
    m_painters.append(new dai::DepthFramePainter);
    m_painters.append(new dai::SkeletonPainter);
    m_initialised = false;
    this->stop();

    //QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(playNextFrame()));
}

InstanceViewer::~InstanceViewer()
{
    /*QListIterator<dai::ViewerPainter*> it(m_painters);

    while (it.hasNext()) {
        dai::ViewerPainter* painter = it.next();
        delete painter;
    }*/

    m_painters.clear();
}

dai::DepthFramePainter& InstanceViewer::getDepthPainter() const
{
    dai::DepthFramePainter* painter = static_cast<dai::DepthFramePainter*>(m_painters.at(0));
    return *painter;
}

dai::SkeletonPainter& InstanceViewer::getSkeletonPainter() const
{
    dai::SkeletonPainter* painter = static_cast<dai::SkeletonPainter*>(m_painters.at(1));
    return *painter;
}

void InstanceViewer::show() {
    QQuickView::show();
}

void InstanceViewer::play(dai::DataInstance* instance)
{
    stop();
    this->setTitle("Instance Viewer (" + instance->getMetadata().getFileName() + ")");
    m_playList.append(instance);

    QListIterator<dai::DataInstance*> it(m_playList);

    while (it.hasNext()) {
        dai::DataInstance* instance = it.next();
        instance->open();
    }

    m_running = true;
    //m_timer.start(66);
    m_time.start();
    QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    qDebug() << "Playing";
}

void InstanceViewer::stop()
{
    QListIterator<dai::DataInstance*> it(m_playList);

    while (it.hasNext()) {
        dai::DataInstance* instance = it.next();
        instance->close();
    }

    m_frames = 0;
    m_fps = 0;
    m_lastTime = 0;
    m_running = false;
    //m_timer.stop();

    emit changeOfStatus();

    qDebug() << "Stopped";
}

void InstanceViewer::playNextFrame()
{
    QListIterator<dai::DataInstance*> it(m_playList);

    m_update_pending = false;

    while (it.hasNext()) {

        dai::DataInstance* instance = it.next();

        if (instance != NULL && instance->hasNext())
        {
            const dai::DataFrame& frame = instance->nextFrame();
            dai::InstanceInfo::InstanceType instanceType = instance->getMetadata().getType();

            if (instanceType == dai::InstanceInfo::Depth)
            {
                const dai::DepthFrame& depthFrame = static_cast<const dai::DepthFrame&>(frame);
                this->getDepthPainter().setFrame(depthFrame);
            }
            else if (instanceType == dai::InstanceInfo::Skeleton)
            {
                const dai::Skeleton skeletonFrame = static_cast<const dai::Skeleton&>(frame);
                this->getSkeletonPainter().setFrame(skeletonFrame);
            }

            //renderOpenGLScene();
            QQuickView::update();
        }
        else if (instance != NULL)
        {
            stop();
            qDebug() << "Closed";
        }
    }
}

void InstanceViewer::renderLater()
{
    if (!m_update_pending) {
        m_update_pending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

void InstanceViewer::initialise()
{
    QListIterator<dai::ViewerPainter*> it(m_painters);

    while (it.hasNext()) {
        dai::ViewerPainter* painter = it.next();
        painter->initialise();
    }

    resetPerspective();
}

void InstanceViewer::renderOpenGLScene()
{
    // First init (depends of video mode)
    // Make here in order to use the OpenGLContext initalised by QtQuick
    if ( !m_initialised ) {
        initialise();
        m_initialised = true;
    }

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

    if (m_running)
    {
        // Compute Frame Per Seconds
        qint64 timeNow = m_time.elapsed();
        m_fps = m_frames / (timeNow / 1000.0f);
        m_frames++;

        // Draw
        QListIterator<dai::ViewerPainter*> it(m_painters);

        while (it.hasNext()) {
            dai::ViewerPainter* painter = it.next();
            painter->render();
        }

        // Emit signal at 30 Hz
        if ( (timeNow - m_lastTime) >= 400) {
            m_lastTime = timeNow;
            emit changeOfStatus();
        }
    }

    // Restore
    glDisable(GL_DEPTH_TEST);
}

void InstanceViewer::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
}

void InstanceViewer::updatePaintersMatrix()
{
    QListIterator<dai::ViewerPainter*> it(m_painters);

    while (it.hasNext()) {
        dai::ViewerPainter* painter = it.next();
        painter->setMatrix(matrix);
    }

    qDebug() << matrix;
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

bool InstanceViewer::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        playNextFrame();
        return true;
    case QEvent::Close:
        this->stop();
        this->destroy();
        emit viewerClose(this);
        return true;
        break;
    default:
        return QQuickView::event(event);
    }
}
