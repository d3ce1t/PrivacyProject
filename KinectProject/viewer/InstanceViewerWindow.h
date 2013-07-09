#ifndef INSTANCEVIEWERWINDOW_H
#define INSTANCEVIEWERWINDOW_H

#include <QQuickWindow>
#include "viewer/PlaybackControl.h"
#include "viewer/InstanceViewer.h"
#include <QQmlApplicationEngine>

namespace dai {

class InstanceViewerWindow : public QObject, public dai::PlaybackControl::PlaybackListener
{
    Q_OBJECT
    Q_PROPERTY(float fps READ getFPS NOTIFY changeOfStatus)

public:
    InstanceViewerWindow();
    virtual ~InstanceViewerWindow();
    void onNewFrame(QList<dai::DataFrame*> dataFrames);
    void onPlaybackStart();
    void onPlaybackStop();
    void setTitle(const QString& title);
    void show();

signals:
    void changeOfStatus();

public slots:
    void processListItem(QListWidget* widget);

private slots:
    void onRenderedFrame();
    float getFPS() const;

private:
    float                      m_fps;
    QQmlApplicationEngine      m_engine;
    InstanceViewer*            m_viewer;
    QQuickWindow*              m_window;
};

} // End Namespace

#endif // INSTANCEVIEWERWINDOW_H
