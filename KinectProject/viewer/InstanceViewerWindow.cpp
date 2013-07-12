#include "InstanceViewerWindow.h"
#include "viewer/InstanceViewer.h"
#include "InstanceWidgetItem.h"
#include "dataset/Dataset.h"
#include "dataset/InstanceInfo.h"
#include "types/UserFrame.h"
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

    connect(m_window, SIGNAL(closing(QQuickCloseEvent*)), this, SLOT(deleteLater()));
    connect(m_viewer, SIGNAL(frameRendered()), this, SLOT(onRenderedFrame()));
    setTitle("Instance Viewer");
}

InstanceViewerWindow::~InstanceViewerWindow()
{
    qDebug() << "InstanceViewerWindow::~InstanceViewerWindow()";
    m_viewer = NULL;
    m_window = NULL;
}

void InstanceViewerWindow::processListItem(QListWidget* widget)
{
    if (widget == NULL)
        return;

    InstanceWidgetItem* instanceItem = (InstanceWidgetItem*) widget->selectedItems().at(0);
    InstanceInfo& info = instanceItem->getInfo();
    const Dataset* dataset = info.parent()->dataset();
    DataInstance* instance = NULL;

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
    playback()->addNewFrameListener(this, instance);
    playback()->play(true);
    setTitle("Instance Viewer (" + instance->getTitle() + ")");
}

void InstanceViewerWindow::onPlaybackStart()
{

}

void InstanceViewerWindow::onPlaybackStop()
{

}

void InstanceViewerWindow::onNewFrame(QList<dai::DataFrame*> dataFrames)
{
    m_fps = playback()->getFPS();
    emit changeOfStatus();

    // Filter
    dataFrames = applyFilters(dataFrames);

    // Sent to viewer
    // I want to execute method in the thread I belong to
    QMetaObject::invokeMethod(m_viewer, "onNewFrame",
                                  Qt::AutoConnection,
                                  Q_ARG(QList<dai::DataFrame*>, dataFrames));
}

QList<DataFrame*> InstanceViewerWindow::applyFilters(QList<DataFrame *> &dataFrames) const
{
    QList<DataFrame*> filteredFrameList;

    UserFrame* userMask = NULL;
    int i = 0;

    // First I get the userframe (if it exists) and apply their filter
    // I will use userframe ad user mask
    while (!userMask && i < dataFrames.size())
    {
        DataFrame* frame = dataFrames.at(i);

        if (frame->getType() == DataFrame::User) {
            userMask = (UserFrame*) applyFilter(frame);
            if (dataFrames.size() == 1) // I only show user mask if it's the only one frame
                filteredFrameList << userMask;
        }

        i++;
    }

    // Filter rest of frames, using userMask
    foreach (DataFrame* inputFrame, dataFrames)
    {
        if (inputFrame->getType() == DataFrame::User) {
            continue;
        }

        filteredFrameList << applyFilter(inputFrame, userMask);
    }

    return filteredFrameList;
}

DataFrame *InstanceViewerWindow::applyFilter(DataFrame* inputFrame, UserFrame* userMask) const
{
    QList<FrameFilter*>* filters = m_filters.value(inputFrame->getType());

    if (filters == NULL)
        return inputFrame;

    // I clone the frame because I do not want to modify the frame read by the instance
    DataFrame* outputFrame = inputFrame->clone();

    foreach (FrameFilter* frameFilter, *filters)
    {
        frameFilter->setMask(userMask);
        frameFilter->applyFilter(outputFrame);
        frameFilter->setMask(NULL); // Hack
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

void InstanceViewerWindow::addFilter(DataFrame::FrameType type, FrameFilter *filter)
{
    QList<FrameFilter*>* filtersList = NULL;

    if (!m_filters.contains(type)) {
        filtersList = new QList<FrameFilter*>;
        m_filters.insert(type, filtersList);
    } else {
        filtersList = m_filters.value(type);
    }

    filtersList->append(filter);
}

void InstanceViewerWindow::enableColorFilter(bool value)
{
    QList<FrameFilter*>* filters = m_filters.value(DataFrame::Color);

    foreach (FrameFilter* filter, *filters) {
        filter->enableFilter(value);
    }
}

void InstanceViewerWindow::show()
{
    if (m_window)
        m_window->show();
}

} // End Namespace
