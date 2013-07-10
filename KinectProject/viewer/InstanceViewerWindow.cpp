#include "InstanceViewerWindow.h"
#include "viewer/InstanceViewer.h"
#include "InstanceWidgetItem.h"
#include "dataset/Dataset.h"
#include "dataset/InstanceInfo.h"
#include <QQmlContext>

namespace dai {

InstanceViewerWindow::InstanceViewerWindow()
{ 
    // Viewer setup
    m_fps = 0;

    // QML Setup
    m_engine.rootContext()->setContextProperty("winObject", (QObject *) this);
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
    releasePlayback();
    m_viewer = NULL;
    m_window = NULL;
}

void InstanceViewerWindow::processListItem(QListWidget* widget)
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
    else if (info.getType() == dai::InstanceInfo::User) {
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
    acquirePlayback();

    // I want to execute method in the thread I belong to
    QMetaObject::invokeMethod(m_viewer, "onNewFrame",
                                  Qt::AutoConnection,
                                  Q_ARG(QList<dai::DataFrame*>, dataFrames));
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

void InstanceViewerWindow::show()
{
    if (m_window)
        m_window->show();
}

} // End Namespace
