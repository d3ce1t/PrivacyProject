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

class InstanceViewerWindow : public PlaybackListener
{
    Q_OBJECT

    Q_PROPERTY(float fps READ getFPS NOTIFY changeOfStatus)
    Q_PROPERTY(bool colorFilter WRITE enableColorFilter)
    Q_PROPERTY(bool blurFilter WRITE enableBlurFilter)

public:
    InstanceViewerWindow();
    virtual ~InstanceViewerWindow();
    void setTitle(const QString& title);
    void addFilter(DataFrame::FrameType type, shared_ptr<FrameFilter> filter);
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
    QHash<DataFrame::FrameType, QList<shared_ptr<FrameFilter>>*> m_filters;
};

} // End Namespace

#endif // INSTANCEVIEWERWINDOW_H
