#ifndef INSTANCEVIEWERWINDOW_H
#define INSTANCEVIEWERWINDOW_H

#include <QQuickWindow>
#include "viewer/PlaybackControl.h"
#include "viewer/InstanceViewer.h"
#include <QQmlApplicationEngine>
#include "filters/FrameFilter.h"
#include <QMultiHash>

namespace dai {

class InstanceViewerWindow : public PlaybackListener
{
    Q_OBJECT

    Q_PROPERTY(float fps READ getFPS NOTIFY changeOfStatus)

public:
    InstanceViewerWindow();
    virtual ~InstanceViewerWindow();
    void setTitle(const QString& title);
    void show();

signals:
    void changeOfStatus();

public slots:
    void processListItem(QListWidget* widget);
    void enableInvisibilityFilter();
    void enableBlurFilter();
    void disableColorFilter();

private slots:
    void onRenderedFrame();
    float getFPS() const;

protected:
    void onNewFrame(const QList<shared_ptr<DataFrame>>& dataFrames);
    void onPlaybackStart();
    void onPlaybackStop();

private:
    QList<shared_ptr<DataFrame>> applyFilters(const QList<shared_ptr<DataFrame> > &dataFrames) const;
    shared_ptr<DataFrame> applyFilter(shared_ptr<DataFrame> inputFrame, shared_ptr<UserFrame> userMask = nullptr) const;

    float                      m_fps;
    QQmlApplicationEngine      m_engine;
    InstanceViewer*            m_viewer;
    QQuickWindow*              m_window;
    QMultiHash<DataFrame::FrameType, shared_ptr<FrameFilter>> m_filters;
    shared_ptr<FrameFilter>    m_activeFilterArray[4];
};

} // End Namespace

#endif // INSTANCEVIEWERWINDOW_H
