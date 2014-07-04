#include "ViewerEngine.h"
#include <QtQml>
#include "filters/DilateUserFilter.h"
#include "playback/PlaybackControl.h"
#include "viewer/InstanceViewer.h"
#include "viewer/Scene2DPainter.h"
#include "viewer/Scene3DPainter.h"
#include "viewer/SilhouetteItem.h"
#include "viewer/SkeletonItem.h"

ViewerEngine::ViewerEngine(ViewerMode mode)
    : QObject()
    , m_quickWindow(nullptr)
    , m_running(false)
{
    qmlRegisterType<ViewerEngine>("ViewerEngine", 1, 0, "ViewerEngine");
    qmlRegisterType<InstanceViewer>("InstanceViewer", 1, 0, "InstanceViewer");
    qmlRegisterUncreatableType<QMLEnumsWrapper>("edu.dai.kinect", 1, 0, "ColorFilter", "This exports SomeState enums to QML");

    qRegisterMetaType<QHashDataFrames>("QHashDataFrames");
    qRegisterMetaType<PlaybackControl*>("PlaybackControl*");
    qRegisterMetaType<const PlaybackControl*>("const PlaybackControl*");
    qRegisterMetaType<QList<shared_ptr<StreamInstance>>>("QList<shared_ptr<StreamInstance>>");
    qRegisterMetaType<shared_ptr<SkeletonFrame>>("shared_ptr<SkeletonFrame>");

    // Create Scene
    if (mode == MODE_3D) {
        m_scene.reset(new Scene3DPainter);
    }
    else {
        m_scene.reset(new Scene2DPainter);
    }

    m_mode = mode;

    // Filters setup
    //shared_ptr<DilateUserFilter> dilateFilter(new DilateUserFilter);
    //dilateFilter->enableFilter(true);

    // Filters are later retrieved from more recently to less recently inserted
    //m_filters.insert(DataFrame::Mask, dilateFilter);
}

ViewerEngine::~ViewerEngine()
{
    //m_filters.clear();
    m_running = false;
    m_quickWindow = nullptr;
    qDebug() << "ViewerEngine::~ViewerEngine()";
}

void ViewerEngine::startEngine(QQuickWindow* window)
{
    m_quickWindow = window;
    m_running = true;
}

bool ViewerEngine::running() const
{
    return m_running;
}

shared_ptr<dai::ScenePainter> ViewerEngine::scene()
{
    return m_scene;
}

void ViewerEngine::onPlusKeyPressed()
{
    emit plusKeyPressed();
}

void ViewerEngine::onMinusKeyPressed()
{
    emit minusKeyPressed();
}

void ViewerEngine::onSpaceKeyPressed()
{
    emit spaceKeyPressed();
}

void ViewerEngine::prepareScene(dai::QHashDataFrames dataFrames)
{
    if (!m_running)
        return;

    shared_ptr<SilhouetteItem> silhouetteItem = static_pointer_cast<SilhouetteItem>(m_scene->getFirstItem(ITEM_SILHOUETTE));
    shared_ptr<SkeletonItem> skeletonItem = static_pointer_cast<SkeletonItem>(m_scene->getFirstItem(ITEM_SKELETON));

    // Clear items of the scene
    m_scene->clearItems();

    // Background of Scene
    if (m_mode == MODE_2D && dataFrames.contains(DataFrame::Color))
    {
        // Compute dilate user mask to separate background from foreground
        if (dataFrames.contains(DataFrame::Mask))
        {
             /*shared_ptr<DilateUserFilter> dilateFilter = static_pointer_cast<DilateUserFilter>(m_filters.value(DataFrame::Mask));
             dilateFilter->setDilationSize(18);
             shared_ptr<MaskFrame> userMask = static_pointer_cast<MaskFrame>(applyFilter(dataFrames.value(DataFrame::Mask)));*/
             shared_ptr<Scene2DPainter> scene = static_pointer_cast<Scene2DPainter>(m_scene);
             scene->setMask(static_pointer_cast<MaskFrame>(dataFrames.value(DataFrame::Mask)));

             // Add silhuette item to the scene
             if (!silhouetteItem)
                 silhouetteItem.reset(new SilhouetteItem);

             silhouetteItem->setUser( static_pointer_cast<MaskFrame>(dataFrames.value(DataFrame::Mask)) );
             m_scene->addItem(silhouetteItem);
        }
        else {
            static int count = 0;
            qDebug() << "Viewer Engine No Silhouette" << count++;
        }

        m_scene->setBackground( dataFrames.value(DataFrame::Color) );
    }
    else if (m_mode == MODE_3D && dataFrames.contains(DataFrame::Depth)) {
        m_scene->setBackground( dataFrames.value(DataFrame::Depth) );
    }

    // Add skeleton item to the scene
    if (dataFrames.contains(DataFrame::Skeleton))
    {
        if (!skeletonItem)
            skeletonItem.reset(new SkeletonItem);

        skeletonItem->setSkeleton( static_pointer_cast<SkeletonFrame>(dataFrames.value(DataFrame::Skeleton)) );
        m_scene->addItem(skeletonItem);
    }

    m_scene->markAsDirty();
}

void ViewerEngine::renderOpenGLScene(QOpenGLFramebufferObject* fbo)
{
    // Draw
    if (m_running)
    {
        m_size = fbo->size();
        m_scene->setSize(m_size.width(), m_size.height());
        m_scene->renderScene(fbo);
        emit frameRendered();
    }
}

/*shared_ptr<DataFrame> ViewerEngine::applyFilter(shared_ptr<DataFrame> inputFrame, shared_ptr<MaskFrame> userMask) const
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
}*/

void ViewerEngine::enableFilter(int filter)
{
    if (m_mode == MODE_2D) {
        shared_ptr<Scene2DPainter> scene = static_pointer_cast<Scene2DPainter>(m_scene);
        scene->enableFilter( (QMLEnumsWrapper::ColorFilter) filter);
    }
}

void ViewerEngine::resetPerspective()
{
    m_scene->resetPerspective();
}

void ViewerEngine::rotateAxisX(float angle)
{
    m_scene->getMatrix().rotate(angle, QVector3D(1, 0, 0));
}

void ViewerEngine::rotateAxisY(float angle)
{
    m_scene->getMatrix().rotate(angle, QVector3D(0, 1, 0));
}

void ViewerEngine::rotateAxisZ(float angle)
{
    m_scene->getMatrix().rotate(angle, QVector3D(0, 0, 1));
}

void ViewerEngine::translateAxisX(float value)
{
    m_scene->getMatrix().translate(value, 0, 0);
}

void ViewerEngine::translateAxisY(float value)
{
    m_scene->getMatrix().translate(0, value, 0);
}

void ViewerEngine::translateAxisZ(float value)
{
    m_scene->getMatrix().translate(0, 0, value);
}
