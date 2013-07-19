#include "InstanceViewerWindow.h"
#include "viewer/InstanceViewer.h"
#include "InstanceWidgetItem.h"
#include "dataset/Dataset.h"
#include "dataset/InstanceInfo.h"
#include "types/UserFrame.h"
#include "filters/InvisibilityFilter.h"
#include "filters/DilateUserFilter.h"
#include "filters/BlurFilter.h"
#include <QQmlContext>

namespace dai {

InstanceViewerWindow::InstanceViewerWindow()
{ 
    // Viewer setup
    m_fps = 0;

    // QML Setup
    m_engine.rootContext()->setContextProperty("viewerWindow", (QObject *) this);
    m_engine.load(QUrl("qrc:///qml/qml/main.qml"));
    QObject *topLevel = m_engine.rootObjects().value(0);
    m_window = qobject_cast<QQuickWindow *>(topLevel);

    if ( !m_window ) {
        qWarning("Error: Your root item has to be a Window.");
        return;
    }

    m_viewer = m_window->findChild<InstanceViewer*>("viewer");

    if (!m_viewer) {
        qWarning("Error: Viewer not found.");
        return;
    }

    // Windows setup
    connect(m_window, SIGNAL(closing(QQuickCloseEvent*)), this, SLOT(deleteLater()));
    connect(m_viewer, SIGNAL(frameRendered()), this, SLOT(onRenderedFrame()));
    setTitle("Instance Viewer");

    // Filters setup
    shared_ptr<BlurFilter> blurFilter(new BlurFilter);
    shared_ptr<InvisibilityFilter> invisibilityFilter(new InvisibilityFilter);
    shared_ptr<DilateUserFilter> dilateFilter(new DilateUserFilter);

    dilateFilter->enableFilter(true);

    // Filters are later retrieved from more recently to less recently inserted
    m_filters.insert(DataFrame::Color, blurFilter);
    m_filters.insert(DataFrame::Color, invisibilityFilter);
    m_filters.insert(DataFrame::User, dilateFilter);
}

InstanceViewerWindow::~InstanceViewerWindow()
{
    qDebug() << "InstanceViewerWindow::~InstanceViewerWindow()";
    m_viewer = nullptr;
    m_window = nullptr;
    m_filters.clear();
}

void InstanceViewerWindow::processListItem(QListWidget* widget)
{
    if (widget == nullptr)
        return;

    InstanceWidgetItem* instanceItem = (InstanceWidgetItem*) widget->selectedItems().at(0);
    InstanceInfo& info = instanceItem->getInfo();
    const Dataset* dataset = info.parent()->dataset();
    shared_ptr<DataInstance> instance;

    if (info.getType() == InstanceInfo::Depth)
        instance = dataset->getDepthInstance(info);
    else if (info.getType() == InstanceInfo::Skeleton) {
        instance = dataset->getSkeletonInstance(info);
    }
    else if (info.getType() == InstanceInfo::Color) {
        instance = dataset->getColorInstance(info);
    }
    else if (info.getType() == InstanceInfo::User) {
        instance = dataset->getUserInstance(info);
    }
    else {
        return;
    }

    playback()->removeListener(this, instance->getType());
    playback()->addInstance(instance);
    playback()->addListener(this, instance);
    playback()->play(true);
    setTitle("Instance Viewer (" + instance->getTitle() + ")");
}

void InstanceViewerWindow::onPlaybackStart()
{

}

void InstanceViewerWindow::onPlaybackStop()
{

}

void InstanceViewerWindow:: onNewFrame(const QList<shared_ptr<DataFrame> > &dataFrames)
{
    m_fps = playback()->getFPS();
    emit changeOfStatus();

    // Filter
    QList<shared_ptr<DataFrame> > filteredFrames = applyFilters(dataFrames);

    // Sent to viewer
    // I want to execute method in the thread I belong to
    QMetaObject::invokeMethod(m_viewer, "onNewFrame",
                                  Qt::AutoConnection,
                                  Q_ARG(QList<shared_ptr<dai::DataFrame>>, filteredFrames));
}

QList<shared_ptr<DataFrame> > InstanceViewerWindow::applyFilters(const QList<shared_ptr<DataFrame>> &dataFrames) const
{
    QList<shared_ptr<DataFrame>> filteredFrameList;
    shared_ptr<UserFrame> userMask;
    int i = 0;

    // HACK: First I get the userframe (if it exists) and apply their filter
    // I will use userframe as user mask
    while (!userMask && i < dataFrames.size())
    {
        shared_ptr<DataFrame> frame = dataFrames.at(i);

        if (frame->getType() == DataFrame::User) {
            userMask = static_pointer_cast<UserFrame>(applyFilter(frame));
            filteredFrameList << userMask;
        }

        i++;
    }

    // Filter rest of frames, using userMask
    foreach (shared_ptr<DataFrame> inputFrame, dataFrames)
    {
        if (inputFrame->getType() == DataFrame::User) {
            continue;
        }

        filteredFrameList << applyFilter(inputFrame, userMask);
    }

    return filteredFrameList;
}

shared_ptr<DataFrame> InstanceViewerWindow::applyFilter(shared_ptr<DataFrame> inputFrame, shared_ptr<UserFrame> userMask) const
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

void InstanceViewerWindow::onRenderedFrame()
{
    releasePlayback();
}

float InstanceViewerWindow::getFPS() const
{
    return m_fps;
}

void InstanceViewerWindow::setTitle(const QString& title)
{
    if (m_window)
        m_window->setTitle(title);
}

void InstanceViewerWindow::enableInvisibilityFilter()
{
    shared_ptr<FrameFilter> filter = m_filters.values(DataFrame::Color).at(0);

    if (m_activeFilterArray[DataFrame::Color])
        m_activeFilterArray[DataFrame::Color]->enableFilter(false);

    m_activeFilterArray[DataFrame::Color] = filter;
    filter->enableFilter(true);
}

void InstanceViewerWindow::enableBlurFilter()
{
    shared_ptr<FrameFilter> filter = m_filters.values(DataFrame::Color).at(1);

    if (m_activeFilterArray[DataFrame::Color])
        m_activeFilterArray[DataFrame::Color]->enableFilter(false);

    m_activeFilterArray[DataFrame::Color] = filter;
    filter->enableFilter(true);
}

void InstanceViewerWindow::disableColorFilter()
{
    m_activeFilterArray[DataFrame::Color]->enableFilter(false);
    m_activeFilterArray[DataFrame::Color] = nullptr;
}

void InstanceViewerWindow::show()
{
    if (m_window)
        m_window->show();
}

} // End Namespace
