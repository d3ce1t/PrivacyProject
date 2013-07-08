#ifndef INSTANCEVIEWERWINDOW_H
#define INSTANCEVIEWERWINDOW_H

#include <QQuickWindow>
#include "viewer/PlaybackControl.h"
#include "viewer/InstanceViewer.h"
#include <QQmlApplicationEngine>

class InstanceViewerWindow : public QObject, public dai::PlaybackControl::PlaybackListener
{
    Q_OBJECT
    Q_PROPERTY(float fps READ getFPS NOTIFY changeOfStatus)

public:
    InstanceViewerWindow();
    virtual ~InstanceViewerWindow();
    void onNewFrame(QList<dai::DataFrame*> dataFrames);
    void setPlayback(dai::PlaybackControl* playback);
    void setTitle(const QString& title);
    void show();

signals:
    void changeOfStatus();

public slots:
    void processListItem(QListWidget* widget);

private slots:
    void acquirePlayback();
    void releasePlayback();
    float getFPS() const;

private:
    qint64                     m_token;
    float                      m_fps;
    QQmlApplicationEngine      m_engine;
    InstanceViewer*            m_viewer;
    QQuickWindow*              m_window;
    dai::PlaybackControl*      m_playback;
};

#endif // INSTANCEVIEWERWINDOW_H
