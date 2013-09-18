#include "InstanceViewer.h"
#include <QDebug>
#include <QQmlContext>
#include <QtQml>
#include <QQuickWindow>
#include <iostream>
#include "viewer/Scene2DPainter.h"
#include "viewer/Scene3DPainter.h"
#include "viewer/SilhouetteItem.h"
#include "viewer/SkeletonItem.h"
#include "filters/DilateUserFilter.h"

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

    shared_ptr<SilhouetteItem> silhouetteItem = static_pointer_cast<SilhouetteItem>(m_scene->getFirstItem(ITEM_SILHOUETTE));
    shared_ptr<SkeletonItem> skeletonItem = static_pointer_cast<SkeletonItem>(m_scene->getFirstItem(ITEM_SKELETON));

    // Clear items of the scene
    m_scene->clearItems();

    // Background of Scene
    if (dataFrames.contains(DataFrame::Color)) {
        // Compute dilate user mask to separate background from foreground
        if (dataFrames.contains(DataFrame::User)) {
             shared_ptr<DilateUserFilter> dilateFilter = static_pointer_cast<DilateUserFilter>(m_filters.value(DataFrame::User));
             dilateFilter->setDilationSize(18);
             shared_ptr<UserFrame> userMask = static_pointer_cast<UserFrame>(applyFilter(dataFrames.value(DataFrame::User)));
             shared_ptr<Scene2DPainter> scene = static_pointer_cast<Scene2DPainter>(m_scene);
             scene->setMask(userMask);

             // Add silhuette item to the scene
             if (!silhouetteItem)
                 silhouetteItem.reset(new SilhouetteItem);

             silhouetteItem->setUser( static_pointer_cast<UserFrame>(dataFrames.value(DataFrame::User)) );
             m_scene->addItem(silhouetteItem);
        }

        m_scene->setBackground( dataFrames.value(DataFrame::Color) );
    }
    else if (dataFrames.contains(DataFrame::Depth)) {
        m_scene->setBackground( dataFrames.value(DataFrame::Depth) );
    }

    // I only show user mask if it's the only one frame
    if (dataFrames.size() > 1)
        dataFrames.remove(DataFrame::User);

    // Add skeleton item to the scene
    if (dataFrames.contains(DataFrame::Skeleton))
    {
        if (!skeletonItem)
            skeletonItem.reset(new SkeletonItem);

        skeletonItem->setSkeleton( static_pointer_cast<SkeletonFrame>(dataFrames.value(DataFrame::Skeleton)) );
        m_scene->addItem(skeletonItem);
    }

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
