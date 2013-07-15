#ifndef INSTANCEVIEWERWINDOW_H
#define INSTANCEVIEWERWINDOW_H

#include <QQuickWindow>
#include "viewer/PlaybackControl.h"
#include "viewer/InstanceViewer.h"
#include <QQmlApplicationEngine>
#include "filters/FrameFilter.h"
#include <QHash>
#include <QList>

namespace dai {

class InstanceViewerWindow : public QObject, public PlaybackControl::PlaybackListener
{
    Q_OBJECT
    Q_PROPERTY(float fps READ getFPS NOTIFY changeOfStatus)
    Q_PROPERTY(bool colorFilter WRITE enableColorFilter)
    Q_PROPERTY(bool blurFilter WRITE enableBlurFilter)

public:
    InstanceViewerWindow();
    virtual ~InstanceViewerWindow();
    void onNewFrame(QList<DataFrame*> dataFrames);
    void onPlaybackStart();
    void onPlaybackStop();
    void setTitle(const QString& title);
    void addFilter(DataFrame::FrameType type, FrameFilter *filter);
    void show();

signals:
    void changeOfStatus();

public slots:
    void processListItem(QListWidget* widget);

private slots:
    void onRenderedFrame();
    float getFPS() const;
    void enableColorFilter(bool value);
    void enableBlurFilter(bool value);

private:
    QList<DataFrame*> applyFilters(QList<DataFrame *>& dataFrames) const;
    DataFrame* applyFilter(DataFrame* inputFrame, UserFrame* userMask = NULL) const;

    float                      m_fps;
    QQmlApplicationEngine      m_engine;
    InstanceViewer*            m_viewer;
    QQuickWindow*              m_window;
    QHash<DataFrame::FrameType, QList<FrameFilter*>*> m_filters;
};

} // End Namespace

#endif // INSTANCEVIEWERWINDOW_H
