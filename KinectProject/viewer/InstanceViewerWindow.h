#ifndef INSTANCEVIEWERWINDOW_H
#define INSTANCEVIEWERWINDOW_H

#include <QQuickWindow>
#include "playback/PlaybackListener.h"
#include "viewer/InstanceViewer.h"
#include <QQmlApplicationEngine>
#include "filters/FrameFilter.h"
#include <QMultiHash>
#include <QObject>
#include <QTableView>
#include "types/Skeleton.h"
#include <QStandardItemModel>

namespace dai {

class InstanceViewerWindow : public QObject, public PlaybackListener
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
    void showJointsWindow();
    void showDistancesWindow();
    void showQuaternionsWindow();
    //void enableInvisibilityFilter();
    //void enableBlurFilter();
    //void disableColorFilter();

private slots:
    float getFPS() const;
    void completeAsyncTask();

protected:
    void onNewFrame(const QHash<DataFrame::FrameType, shared_ptr<DataFrame>>& dataFrames);
    void onPlaybackStart();
    void onPlaybackStop();

private:
    shared_ptr<DataFrame> applyFilter(shared_ptr<DataFrame> inputFrame, shared_ptr<UserFrame> userMask = nullptr) const;
    void setupJointsModel(QStandardItemModel &model);
    void setupDistancesModel(QStandardItemModel &model);
    void setupQuaternionModel(QStandardItemModel &model);
    void feedJointsModel(const Skeleton &skeleton, QStandardItemModel &model);
    void feedDistancesModel(const Skeleton& skeleton, QStandardItemModel& model);
    void feedQuaternionsModel(const Skeleton &skeleton, QStandardItemModel& model);
    float colorIntensity(float value);

    float                      m_fps;
    QQmlApplicationEngine      m_engine;
    InstanceViewer*            m_viewer;
    QQuickWindow*              m_window;
    QMultiHash<DataFrame::FrameType, shared_ptr<FrameFilter>> m_filters;
    shared_ptr<FrameFilter>    m_activeFilterArray[4];

    // Windows and models for Skeleton data
    QTableView                 m_joints_table_view;
    QTableView                 m_distances_table_view;
    QTableView                 m_quaternions_table_view;
    QStandardItemModel         m_joints_model;
    QStandardItemModel         m_distances_model;
    QStandardItemModel         m_quaternions_model;
    Quaternion                 m_lastQuaternions[20];
    long                       m_frameCounter;
};

} // End Namespace

#endif // INSTANCEVIEWERWINDOW_H
