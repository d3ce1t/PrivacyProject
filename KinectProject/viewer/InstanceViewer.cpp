#include "InstanceViewer.h"
#include "dataset/Dataset.h"
#include <QDebug>
#include <QQmlContext>
#include <QtQml>
#include <iostream>
#include "../dataset/InstanceInfo.h"
#include "DepthFramePainter.h"
#include "SkeletonPainter.h"
#include "ColorFramePainter.h"
#include "InstanceWidgetItem.h"


InstanceViewer::InstanceViewer( QWindow *parent )
    : QQuickView( parent )
{
    // QML Setup
    rootContext()->setContextProperty("winObject", (QObject *) this);
    QQuickView::setSource(QUrl("qrc:///qml/qml/main.qml"));
    setClearBeforeRendering( false );
    connect(this, SIGNAL(beforeRendering()), SLOT(renderOpenGLScene()), Qt::DirectConnection);
    connect(this, SIGNAL(closing(QQuickCloseEvent*)), this, SLOT(deleteLater()));

    // Viewer Setup
    this->setTitle("Instance Viewer");
    m_painters.insert(dai::DataFrame::Color, new dai::ColorFramePainter(NULL));
    m_painters.insert(dai::DataFrame::Depth, new dai::DepthFramePainter(NULL));
    m_painters.insert(dai::DataFrame::Skeleton, new dai::SkeletonPainter(NULL));

    m_running = false;
    m_playback = NULL;
    m_fps = 0.0;
    m_token = -1;
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
    m_playback->removeListener(this);
    m_playback->release(this, m_token);

    qDebug() << "InstanceViewer::~InstanceViewer()";
}

void InstanceViewer::show() {
    QQuickView::show();
}

void InstanceViewer::setPlayback(dai::PlaybackControl* playback)
{
    m_playback = playback;
}

void InstanceViewer::onNewFrame(QList<dai::DataFrame*> dataFrames)
{
    // I want to execute method in the thread I belong to
    QMetaObject::invokeMethod(this, "onNewFrameAux",
                                  Qt::AutoConnection,
                                  Q_ARG(QList<dai::DataFrame*>, dataFrames));
}

void InstanceViewer::onNewFrameAux(QList<dai::DataFrame*> dataFrames)
{
    m_fps = m_playback->getFPS();
    m_running = true;

    m_mutex.lock();
    m_token = m_playback->acquire(this);

    foreach (dai::DataFrame* frame, dataFrames) {
        dai::Painter* painter = m_painters.value(frame->getType());
        painter->prepareData(frame);
    }

    m_mutex.unlock();

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
    if (m_running)
    {
        m_mutex.lock();

        foreach (dai::Painter* painter, m_painters.values()) {
            painter->renderNow();
        }

        if (m_token != -1) {
            m_playback->release(this, m_token);
            m_token = -1;
        }

        m_mutex.unlock();
        emit changeOfStatus();
    }

    // Restore
    glDisable(GL_DEPTH_TEST);
}

void InstanceViewer::processListItem(QListWidget* widget)
{
    if (widget == NULL)
        return;

    dai::InstanceWidgetItem* instanceItem = (dai::InstanceWidgetItem*) widget->selectedItems().at(0);
    dai::InstanceInfo& info = instanceItem->getInfo();
    const dai::Dataset* dataset = info.parent()->dataset();
    dai::DataInstance* instance = NULL;

    if (info.getType() == dai::InstanceInfo::Depth)
        instance = dataset->getDepthInstance(info);
    else if (info.getType() == dai::InstanceInfo::Skeleton) {
        instance = dataset->getSkeletonInstance(info);
    }
    else if (info.getType() == dai::InstanceInfo::Color) {
        instance = dataset->getColorInstance(info);
    }
    else {
        return;
    }

    m_playback->addInstance(instance);
    m_playback->removeListener(this, instance->getType());
    m_playback->addNewFrameListener(this, instance);
    m_playback->play(true);

    this->setTitle("Instance Viewer (" + instance->getTitle() + ")");
}

void InstanceViewer::updatePaintersMatrix()
{
    m_mutex.lock();
    foreach (dai::Painter* painter, m_painters.values()) {
        painter->setMatrix(m_matrix);
    }
    m_mutex.unlock();
    QQuickView::update();
}

void InstanceViewer::resetPerspective()
{
    m_matrix.setToIdentity();
    m_matrix.perspective(70, 1.0, 0.01, 10.0);
    m_matrix.translate(0, 0, -1.5);
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

float InstanceViewer::getFPS() const
{
    return m_fps;
}
