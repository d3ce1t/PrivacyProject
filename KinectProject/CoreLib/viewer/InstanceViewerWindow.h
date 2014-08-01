#ifndef INSTANCEVIEWERWINDOW_H
#define INSTANCEVIEWERWINDOW_H

#include <QQuickWindow>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "types/SkeletonFrame.h"
#include <QObject>
#include <QStandardItemModel>
#include <QMutexLocker>
#include "playback/PlaybackControl.h"
#include "playback/FrameListener.h"
#include "viewer/ViewerEngine.h"
#include <QListWidget>
#include <QTableView>

namespace dai {

class InstanceViewerWindow : public QObject, public FrameListener
{
    Q_OBJECT

    Q_PROPERTY(float fps READ getFPS NOTIFY changeOfStatus)

public:
    InstanceViewerWindow();
    virtual ~InstanceViewerWindow();
    const ViewerEngine* viewerEngine() const;
    void setTitle(const QString& title);
    void show();
    QQmlApplicationEngine& qmlEngine();
    QQuickWindow* quickWindow();
    void newFrames(const QHashDataFrames dataFrames) override;
    void setDelay(qint64 milliseconds);
    void setDrawMode(ViewerEngine::DrawMode mode);

signals:
    void changeOfStatus();

public slots:
    void processListItem(QListWidget* widget);
    void showJointsWindow();
    void showDistancesWindow();
    void showQuaternionsWindow();

private slots:
    float getFPS() const;
    void feedDataModels(shared_ptr<SkeletonFrame> skeletonFrame);

private:
    void measureTime(qint64 initialTime);
    void setupJointsModel(QStandardItemModel &model);
    void setupDistancesModel(QStandardItemModel &model);
    void setupQuaternionModel(QStandardItemModel &model);
    void feedJointsModel(const dai::Skeleton &skeleton, QStandardItemModel &model);
    void feedDistancesModel(const dai::Skeleton& skeleton, QStandardItemModel& model);
    void feedQuaternionsModel(const dai::Skeleton &skeleton, QStandardItemModel& model);
    float colorIntensity(float value);

    bool                    m_initialised;
    float                   m_fps;
    long                    m_frameCounter;
    unsigned long           m_delayInMs;

    ViewerEngine*           m_viewerEngine;
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
    bool                    m_modelsInitialised;
    QMutex                  m_modelsLock;
};

} // End Namespace

#endif // INSTANCEVIEWERWINDOW_H
