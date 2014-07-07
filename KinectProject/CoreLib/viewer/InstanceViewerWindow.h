#ifndef INSTANCEVIEWERWINDOW_H
#define INSTANCEVIEWERWINDOW_H

#include <QQuickWindow>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "types/SkeletonFrame.h"
#include <QObject>
#include <QTableView>
#include <QStandardItemModel>
#include <QListWidget>
#include <QMutexLocker>
#include "playback/PlaybackControl.h"
#include "playback/FrameListener.h"
#include "viewer/ViewerEngine.h"

namespace dai {

class InstanceViewerWindow : public QObject, public FrameListener
{
    Q_OBJECT

    Q_PROPERTY(float fps READ getFPS NOTIFY changeOfStatus)

public:

    inline static void initResources() { Q_INIT_RESOURCE(corelib); }

    InstanceViewerWindow();
    virtual ~InstanceViewerWindow();
    void initialise();
    const ViewerEngine* viewerEngine() const;
    void setTitle(const QString& title);
    void show();
    QQmlApplicationEngine& qmlEngine();
    QQuickWindow* quickWindow();
    void newFrames(const QHashDataFrames dataFrames) override;
    void setDelay(qint64 milliseconds);

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
    void feedJointsModel(const Skeleton &skeleton, QStandardItemModel &model);
    void feedDistancesModel(const Skeleton& skeleton, QStandardItemModel& model);
    void feedQuaternionsModel(const Skeleton &skeleton, QStandardItemModel& model);
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
