#include "InstanceViewer.h"
#include <QDebug>
#include <QQmlContext>
#include <QtQml>
#include <QQuickWindow>
#include <iostream>
#include "viewer/Scene2DPainter.h"
#include "viewer/Scene3DPainter.h"
#include "viewer/SilhouetteItem.h"
#include "SkeletonFramePainter.h"
#include "UserFramePainter.h"

InstanceViewer::InstanceViewer()
{
    // QML Setup
    connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(handleWindowChanged(QQuickWindow*)));

    m_running = false;
    m_window = nullptr;

    // Filters setup
    shared_ptr<DilateUserFilter> dilateFilter(new DilateUserFilter);
    dilateFilter->enableFilter(true);

    // Filters are later retrieved from more recently to less recently inserted
    m_filters.insert(DataFrame::User, dilateFilter);
}

InstanceViewer::~InstanceViewer()
{
    m_filters.clear();
    m_running = false;
    m_window = nullptr;
    qDebug() << "InstanceViewer::~InstanceViewer()";
}

void InstanceViewer::setMode(ViewerMode mode)
{
    if (mode == MODE_3D) {
        m_scene.reset(new Scene3DPainter);
    }
    else {
        m_scene.reset(new Scene2DPainter);
    }

    resetPerspective();

    m_mode = mode;
}

ViewerMode InstanceViewer::getMode() const
{
    return m_mode;
}

void InstanceViewer::onNewFrame(QHashDataFrames dataFrames)
{
    m_running = true;
    shared_ptr<UserFrame> userMask1;
    shared_ptr<SilhouetteItem> silhouetteItem = static_pointer_cast<SilhouetteItem>(m_scene->getFirstItem(ITEM_SILHOUETTE));

    // Get UserFrame in order to use as mask
    if (dataFrames.contains(DataFrame::User))
    {
        shared_ptr<DilateUserFilter> dilateFilter = static_pointer_cast<DilateUserFilter>(m_filters.value(DataFrame::User));
        dilateFilter->setDilationSize(18);
        userMask1 = static_pointer_cast<UserFrame>(applyFilter(dataFrames.value(DataFrame::User)));
        shared_ptr<UserFrame> userMask2 = static_pointer_cast<UserFrame>(dataFrames.value(DataFrame::User));

        if (dataFrames.size() > 1) // I only show user mask if it's the only one frame
            dataFrames.remove(DataFrame::User);

        if (silhouetteItem)
            silhouetteItem->setUser(userMask2);
        else
            silhouetteItem.reset(new SilhouetteItem);
    }

    // Then apply filters to the rest of frames
    shared_ptr<DataFrame> bg;

    if (dataFrames.contains(DataFrame::Color))
    {
        shared_ptr<Scene2DPainter> scene = static_pointer_cast<Scene2DPainter>(m_scene);
        scene->setMask(userMask1);
        bg = dataFrames.value(DataFrame::Color);
    }
    else if (dataFrames.contains(DataFrame::Depth)) {
        bg = dataFrames.value(DataFrame::Depth);
    }

    m_scene->clearItems();

    if (silhouetteItem)
        m_scene->addItem(silhouetteItem);

    m_scene->setBackground(bg);

    /*foreach (shared_ptr<dai::DataFrame> inputFrame, dataFrames)
    {
        //m_scene->addItem();

        dai::Painter* painter = m_paintersIndex.value(inputFrame->getType());

        if (painter) {
            painter->setMask1(userMask1);
            painter->setMask2(userMask2);
            painter->prepareData(inputFrame);
        }
    }*/

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
    if (m_mode == MODE_2D) {
        shared_ptr<Scene2DPainter> scene = static_pointer_cast<Scene2DPainter>(m_scene);
        scene->enableFilter( (QMLEnumsWrapper::ColorFilter) filter);
    }
}

void InstanceViewer::renderOpenGLScene()
{
    // Draw
    if (m_running)
    {
        m_scene->setSize(width(), height());
        m_scene->renderScene();
        emit frameRendered();
    }
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

void InstanceViewer::resetPerspective()
{
    m_scene->resetPerspective();
}

void InstanceViewer::rotateAxisX(float angle)
{
    m_scene->getMatrix().rotate(angle, QVector3D(1, 0, 0));
    m_scene->updateItemsMatrix();
}

void InstanceViewer::rotateAxisY(float angle)
{
    m_scene->getMatrix().rotate(angle, QVector3D(0, 1, 0));
    m_scene->updateItemsMatrix();
}

void InstanceViewer::rotateAxisZ(float angle)
{
    m_scene->getMatrix().rotate(angle, QVector3D(0, 0, 1));
    m_scene->updateItemsMatrix();
}

void InstanceViewer::translateAxisX(float value)
{
    m_scene->getMatrix().translate(value, 0, 0);
    m_scene->updateItemsMatrix();
}

void InstanceViewer::translateAxisY(float value)
{
    m_scene->getMatrix().translate(0, value, 0);
    m_scene->updateItemsMatrix();
}

void InstanceViewer::translateAxisZ(float value)
{
    m_scene->getMatrix().translate(0, 0, value);
    m_scene->updateItemsMatrix();
}
