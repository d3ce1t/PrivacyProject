#ifndef INSTANCEVIEWERWINDOW_H
#define INSTANCEVIEWERWINDOW_H

#include <QQuickWindow>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "types/SkeletonFrame.h"
#include <QMultiHash>
#include <QObject>
#include <QTableView>
#include <QStandardItemModel>
#include <QListWidget>
#include "playback/PlaybackControl.h"
#include "viewer/ViewerEngine.h"

namespace dai {

class InstanceViewerWindow : public QObject
{
    Q_OBJECT

    Q_PROPERTY(float fps READ getFPS NOTIFY changeOfStatus)

public:
    InstanceViewerWindow(ViewerMode mode);
    void initialise();
    virtual ~InstanceViewerWindow();
    const ViewerEngine* viewerEngine() const;
    void setTitle(const QString& title);
    void show();
    QQmlApplicationEngine& qmlEngine();
    QQuickWindow* quickWindow();

signals:
    void changeOfStatus();

public slots:
    void processListItem(QListWidget* widget);
    void showJointsWindow();
    void showDistancesWindow();
    void showQuaternionsWindow();
    void newFrames(const QMultiHashDataFrames dataFrames, const qint64 frameId, const PlaybackControl* playback);

private slots:
    float getFPS() const;
    void feedDataModels(shared_ptr<SkeletonFrame> skeletonFrame);

private:
    void setupJointsModel(QStandardItemModel &model);
    void setupDistancesModel(QStandardItemModel &model);
    void setupQuaternionModel(QStandardItemModel &model);
    void feedJointsModel(const Skeleton &skeleton, QStandardItemModel &model);
    void feedDistancesModel(const Skeleton& skeleton, QStandardItemModel& model);
    void feedQuaternionsModel(const Skeleton &skeleton, QStandardItemModel& model);
    float colorIntensity(float value);

    bool                    m_initialised;
    float                   m_fps;

    ViewerEngine*           m_viewerEngine;
    ViewerMode              m_viewerMode;
    QQmlApplicationEngine   m_qmlEngine;
    QQuickWindow*           m_quickWindow;

    // Windows and models for Skeleton data
    QTableView              m_joints_table_view;
    QTableView              m_distances_table_view;
    QTableView              m_quaternions_table_view;
    QStandardItemModel      m_joints_model;
    QStandardItemModel      m_distances_model;
    QStandardItemModel      m_quaternions_model;
    Quaternion              m_lastQuaternions[20];
    long                    m_frameCounter;
};

} // End Namespace

#endif // INSTANCEVIEWERWINDOW_H
