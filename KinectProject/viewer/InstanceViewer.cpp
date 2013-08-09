#include "InstanceViewer.h"
#include <QDebug>
#include <QQmlContext>
#include <QtQml>
#include <QQuickWindow>
#include <iostream>
#include "DepthFramePainter.h"
#include "SkeletonFramePainter.h"
#include "ColorFramePainter.h"
#include "UserFramePainter.h"

InstanceViewer::InstanceViewer()
{
    // QML Setup
    connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(handleWindowChanged(QQuickWindow*)));

    // Create Painters
    dai::ColorFramePainter* color = new dai::ColorFramePainter(this);
    dai::DepthFramePainter* depth = new dai::DepthFramePainter(this);
    dai::SkeletonFramePainter* skeleton = new dai::SkeletonFramePainter(this);
    dai::UserFramePainter* user = new dai::UserFramePainter(this);

    // Store Painters in order print order
    m_painters.append(color);
    m_painters.append(depth);
    m_painters.append(skeleton);
    m_painters.append(user);

    // Index Painters
    m_paintersIndex.insert(dai::DataFrame::Color, color);
    m_paintersIndex.insert(dai::DataFrame::Depth, depth);
    m_paintersIndex.insert(dai::DataFrame::Skeleton, skeleton);
    m_paintersIndex.insert(dai::DataFrame::User, user);

    //m_dummyPainter.reset(new dai::DummyPainter(this));

    m_running = false;
    m_window = nullptr;
    resetPerspective();

    // Filters setup
    shared_ptr<DilateUserFilter> dilateFilter(new DilateUserFilter);
    dilateFilter->enableFilter(true);

    // Filters are later retrieved from more recently to less recently inserted
    m_filters.insert(DataFrame::User, dilateFilter);
}

InstanceViewer::~InstanceViewer()
{
    m_mutex.lock();
    foreach (dai::Painter* painter, m_painters) {
        delete painter;
    }
    m_painters.clear();
    m_paintersIndex.clear();
    m_mutex.unlock();
    m_filters.clear();
    m_running = false;
    m_window = nullptr;
    qDebug() << "InstanceViewer::~InstanceViewer()";
}

void InstanceViewer::onNewFrame(QHashDataFrames dataFrames)
{
    m_running = true;
    shared_ptr<UserFrame> userMask1;
    shared_ptr<UserFrame> userMask2;

    // Get UserFrame in order to use as mask
    if (dataFrames.contains(DataFrame::User))
    {
        shared_ptr<DilateUserFilter> dilateFilter = static_pointer_cast<DilateUserFilter>(m_filters.value(DataFrame::User));

        dilateFilter->setDilationSize(18);
        userMask1 = static_pointer_cast<UserFrame>(applyFilter(dataFrames.value(DataFrame::User)));
        //dilateFilter->setDilationSize(5);
        userMask2 = static_pointer_cast<UserFrame>(dataFrames.value(DataFrame::User));

        if (dataFrames.size() > 1) // I only show user mask if it's the only one frame
            dataFrames.remove(DataFrame::User);
    }

    // Then apply filters to the rest of frames
    m_mutex.lock();

    foreach (shared_ptr<dai::DataFrame> inputFrame, dataFrames)
    {
        dai::Painter* painter = m_paintersIndex.value(inputFrame->getType());

        if (painter) {
            painter->setMask1(userMask1);
            painter->setMask2(userMask2);
            painter->prepareData(inputFrame);
        }
    }

    m_mutex.unlock();

    if (m_window != nullptr)
        m_window->update();
}

shared_ptr<DataFrame> InstanceViewer::applyFilter(shared_ptr<DataFrame> inputFrame, shared_ptr<UserFrame> userMask) const
{
    QList<shared_ptr<FrameFilter>> filters = m_filters.values(inputFrame->getType());

    if (filters.count() == 0)
        return inputFrame;

    // I clone the frame because I do not want to modify the frame read by the instance
    shared_ptr<DataFrame> outputFrame = inputFrame->clone();

    foreach (shared_ptr<FrameFilter> frameFilter, filters)
    {
        frameFilter->setMask(userMask);
        frameFilter->applyFilter(outputFrame);
        frameFilter->setMask(nullptr); // Hack
    }

    return outputFrame;
}

void InstanceViewer::enableFilter(int filter)
{
    dai::ColorFramePainter* painter =  (dai::ColorFramePainter*) m_paintersIndex.value(dai::DataFrame::Color);
    painter->enableFilter( (QMLEnumsWrapper::ColorFilter) filter);
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

    //m_dummyPainter->renderNow();


    // Draw
    if (m_running)
    {
        m_mutex.lock();
        foreach (dai::Painter* painter, m_painters) {
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
    foreach (dai::Painter* painter, m_painters) {
        painter->setMatrix(m_matrix);
    }
    m_mutex.unlock();

    if (m_window != nullptr)
        m_window->update();
}

void InstanceViewer::resetPerspective()
{
    m_matrix.setToIdentity();
    m_matrix.perspective(43, 640/480, 0.01, 50.0);
    //m_matrix.translate(0, 0, 0.2);
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
