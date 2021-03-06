#include "InstanceViewerWindow.h"
#include "playback/PlaybackControl.h"
#include "InstanceWidgetItem.h"
#include "dataset/Dataset.h"
#include "dataset/InstanceInfo.h"
#include "types/MaskFrame.h"
#include "types/MetadataFrame.h"
#include "CustomItem.h"
#include <QQmlContext>
#include <QListWidget>
#include <QMetaEnum>
#include <QDebug>
#include "InstanceViewer.h"


namespace dai {

InstanceViewerWindow::InstanceViewerWindow()
    : m_initialised(false)
    , m_fps(0)
    , m_frameCounter(0)
    , m_delayInMs(0)
    , m_viewerEngine(nullptr)
    , m_quickWindow(nullptr)
    , m_modelsInitialised(false)
{
    m_viewerEngine = new ViewerEngine;

    // expose objects as QML globals
    m_qmlEngine.rootContext()->setContextProperty("Window", this);
    m_qmlEngine.rootContext()->setContextProperty("ViewerEngine", m_viewerEngine);

    // Load QML app
    m_qmlEngine.load(QUrl("qrc:///qml/qml/viewer.qml"));

    // Get Window
    QObject *topLevel = m_qmlEngine.rootObjects().value(0);
    m_quickWindow = qobject_cast<QQuickWindow *>(topLevel);

    if ( !m_quickWindow ) {
        qWarning("Error: Your root item has to be a Window.");
        return;
    }

    m_quickWindow->setTitle("Instance Viewer");
    m_viewerEngine->startEngine(m_quickWindow);

    // Windows setup
    connect(m_quickWindow, SIGNAL(closing(QQuickCloseEvent*)), this, SLOT(deleteLater()));
    m_initialised = true;
}

InstanceViewerWindow::~InstanceViewerWindow()
{
    stopListener();

    // Close windows and clear models
    m_modelsLock.lock();

    m_joints_table_view.close();
    m_joints_model.clear();

    m_distances_table_view.close();
    m_distances_model.clear();

    m_quaternions_table_view.close();
    m_quaternions_model.clear();

    m_modelsLock.unlock();
    qDebug() << "InstanceViewerWindow::~InstanceViewerWindow()";
}

QQmlApplicationEngine& InstanceViewerWindow::qmlEngine()
{
    return m_qmlEngine;
}

QQuickWindow* InstanceViewerWindow::quickWindow()
{
    return m_quickWindow;
}

void InstanceViewerWindow::setTitle(const QString& title)
{
    if (m_quickWindow)
        m_quickWindow->setTitle(title);
}

void InstanceViewerWindow::show()
{
    if (m_quickWindow)
        m_quickWindow->show();
}

void InstanceViewerWindow::processListItem(QListWidget* widget)
{
    Q_UNUSED(widget)
    /*if (widget == nullptr)
        return;

    InstanceWidgetItem* instanceItem = (InstanceWidgetItem*) widget->selectedItems().at(0);
    InstanceInfo& info = instanceItem->getInfo();
    const Dataset& dataset = info.parent().dataset();
    shared_ptr<BaseInstance> instance = dataset.getInstance(info);

    if (instance) {
        playback()->addInstance(instance);
        playback()->addListener(this, instance);
        playback()->play(true);
        setTitle("Instance Viewer (" + instance->getTitle() + ")");
    }*/
}

void InstanceViewerWindow::setDelay(qint64 milliseconds)
{
    m_delayInMs = milliseconds;
}

void InstanceViewerWindow::setDrawMode(ViewerEngine::DrawMode mode)
{
    m_viewerEngine->setDrawMode(mode);
}

void InstanceViewerWindow::showFrame(shared_ptr<ColorFrame> frame)
{
    // Do task
    QHashDataFrames frames;
    frames.insert(DataFrame::Color, frame);
    m_viewerEngine->prepareScene(frames);
    //m_fps = producerHandler()->getFrameRate();
    //emit changeOfStatus();
}

void InstanceViewerWindow::newFrames(const QHashDataFrames dataFrames)
{
    // Copy frames (1 ms)
    QHashDataFrames copyFrames;

    foreach (DataFrame::FrameType key, dataFrames.keys()) {
        shared_ptr<DataFrame> frame = dataFrames.value(key);
        copyFrames.insert(key, frame->clone());
    }

    // Check if the frames has been copied correctly
    if (!hasExpired())
    {
        // Do task
        m_viewerEngine->prepareScene(copyFrames);

        // Feed skeleton data models
        if (copyFrames.contains(DataFrame::Skeleton)) {
            shared_ptr<SkeletonFrame> skeleton = static_pointer_cast<SkeletonFrame>( copyFrames.value(DataFrame::Skeleton) );
            feedDataModels(skeleton);
        }

        if (m_delayInMs > 0)
            QThread::currentThread()->msleep(m_delayInMs);
    }
    else {
        qDebug() << "InstanceViewerWindow - Frame copied out of time";
    }

    m_fps = producerHandler()->getFrameRate();
    emit changeOfStatus();
}

float InstanceViewerWindow::getFPS() const
{
    return m_fps;
}

const ViewerEngine* InstanceViewerWindow::viewerEngine() const
{
    return m_viewerEngine;
}

void InstanceViewerWindow::showJointsWindow()
{
    // Setup
    if (m_joints_table_view.model() != &m_joints_model) {
        setupJointsModel(m_joints_model);
    }

    m_joints_table_view.show();
}

void InstanceViewerWindow::showDistancesWindow()
{
    // Setup
    if (m_distances_table_view.model() != &m_distances_model) {
        setupDistancesModel(m_distances_model);
    }

    m_distances_table_view.show();
}

void InstanceViewerWindow::showQuaternionsWindow()
{
    // Setup
    if (m_quaternions_table_view.model() != &m_quaternions_model) {
        setupQuaternionModel(m_quaternions_model);
    }

    m_quaternions_table_view.show();
}

void InstanceViewerWindow::setupJointsModel(QStandardItemModel &model)
{
    QMutexLocker locker(&m_modelsLock);

    // Setup Joints Model
    model.setRowCount(20);
    model.setColumnCount(3);
    m_joints_table_view.setWindowTitle("Joints info");
    m_joints_table_view.setModel(&model);
    m_joints_table_view.setMinimumSize(460, 630);

    //const QMetaObject &metaObject = dai::SkeletonJoint::staticMetaObject;
    //int index = metaObject.indexOfEnumerator("JointType"); // watch out during refactorings
    //QMetaEnum metaEnum = metaObject.enumerator(index);

    QStringList list = {"pos X", "pos Y", "pos Z"};
    model.setHorizontalHeaderLabels(list);
    list.clear();

    for (int i=0; i<20; ++i) {
        //QString name(metaEnum.valueToKey(i));
        //list << name.mid(6);
        list << "HEAD" << "CENTER_SHOULDER" << "L.SHOULDER" <<
                "R.SHOULDER" << "L.ELBOW" << "R.ELBOW" <<
                "L.WRIST" << "R.WRIST" << "L.HAND" <<
                "R.HAND" << "SPINE" << "CENTER HIP" << "L.HIP" <<
                "R.HIP" << "L.KNEE" << "R.KNEE" << "L.ANKLE" <<
                "R.ANKLE" << "L.FOOT" << "R.FOOT";

    }

    for (int i=0; i<20; ++i) {
        QStandardItem *item = new QStandardItem;
        item->setText(list.at(i));
        model.setVerticalHeaderItem(i, item);
        for (int j=0; j<3; ++j) {
            CustomItem *item = new CustomItem;
            model.setItem(i, j, item);
        }
    }
}

void InstanceViewerWindow::setupDistancesModel(QStandardItemModel &model)
{
    QMutexLocker locker(&m_modelsLock);

    // Setup Distances model
    model.setRowCount(20);
    model.setColumnCount(20);
    m_distances_table_view.setWindowTitle("Distances info");
    m_distances_table_view.setModel(&model);
    m_distances_table_view.setMinimumSize(600, 640);

    //const QMetaObject &skeletonJointMetaObject = SkeletonJoint::staticMetaObject;
    //int index = skeletonJointMetaObject.indexOfEnumerator("JointType"); // watch out during refactorings
    //QMetaEnum metaEnum = skeletonJointMetaObject.enumerator(index);

    QStringList list;

    for (int i=0; i<20; ++i) {
        //QString name(metaEnum.valueToKey(i));
        //list << name.mid(6);
        list << "HEAD" << "CENTER_SHOULDER" << "L.SHOULDER" <<
                "R.SHOULDER" << "L.ELBOW" << "R.ELBOW" <<
                "L.WRIST" << "R.WRIST" << "L.HAND" <<
                "R.HAND" << "SPINE" << "CENTER HIP" << "L.HIP" <<
                "R.HIP" << "L.KNEE" << "R.KNEE" << "L.ANKLE" <<
                "R.ANKLE" << "L.FOOT" << "R.FOOT";
    }

    model.setHorizontalHeaderLabels(list);

    for (int i=0; i<20; ++i) {
        QStandardItem *item = new QStandardItem;
        item->setText(list.at(i));
        model.setVerticalHeaderItem(i, item);
        for (int j=0; j<20; ++j) {
            CustomItem *item = new CustomItem;
            model.setItem(i, j, item);
        }
    }
}

void InstanceViewerWindow::setupQuaternionModel(QStandardItemModel &model)
{
    QMutexLocker locker(&m_modelsLock);

    // Setup Quaternions Model
    model.setRowCount(20);
    model.setColumnCount(5);
    m_quaternions_table_view.setWindowTitle("Quaternions info");
    m_quaternions_table_view.setModel(&model);
    m_quaternions_table_view.setMinimumSize(510, 545);

    //const QMetaObject &quaternionMetaObject = Quaternion::staticMetaObject;
    //int index = quaternionMetaObject.indexOfEnumerator("QuaternionType");
    //QMetaEnum metaEnum = quaternionMetaObject.enumerator(index);

    QStringList list = {"Tensor", "pos X", "pos Y", "pos Z", "angle"};
    model.setHorizontalHeaderLabels(list);
    list.clear();

    for (int i=0; i<20; ++i) {
        //QString name(metaEnum.valueToKey(i));
        //list << name.mid(11);
        list << "Q1" << "Q2" << "Q3" << "Q4" << "Q5" << "Q6" <<
                "Q7" << "Q8" << "Q9" << "Q10" << "Q11" << "Q12" <<
                "Q13" << "Q14" << "Q15" << "Q16" << "Q17" << "Q18" <<
                "Q19" << "Q20" << "Q21" << "Q22";
    }

    for (int i=0; i<20; ++i) {
        QStandardItem *item = new QStandardItem;
        item->setText(list.at(i));
        model.setVerticalHeaderItem(i, item);
        for (int j=0; j<5; ++j) {
            QStandardItem *item = new QStandardItem;
            model.setItem(i, j, item);
        }
    }
}

void InstanceViewerWindow::feedDataModels(shared_ptr<SkeletonFrame> skeletonFrame)
{
    if (!skeletonFrame)
        return;

    QList<int> allUsers = skeletonFrame->getAllUsersId();

    if (!allUsers.isEmpty()) {

        int userId = allUsers.first();
        const auto& skeleton = *(skeletonFrame->getSkeleton(userId));

        if (m_joints_table_view.isVisible())
            feedJointsModel(skeleton, m_joints_model);

        if (m_distances_table_view.isVisible())
            feedDistancesModel(skeleton, m_distances_model);

        if (m_quaternions_table_view.isVisible())
            feedQuaternionsModel(skeleton, m_quaternions_model);
    }
}

void InstanceViewerWindow::feedJointsModel(const dai::Skeleton& skeleton, QStandardItemModel& model)
{
    QMutexLocker locker(&m_modelsLock);

    // Joints Model
    for (int i=0; i<skeleton.getJointsCount(); ++i)
    {
        const SkeletonJoint& joint = skeleton.getJoint( (SkeletonJoint::JointType) i );

        CustomItem* itemX = dynamic_cast<CustomItem*>(model.item(i, 0));
        CustomItem* itemY = dynamic_cast<CustomItem*>(model.item(i, 1));
        CustomItem* itemZ = dynamic_cast<CustomItem*>(model.item(i, 2));

        itemX->setNumber(joint.getPosition().val(0));
        itemY->setNumber(joint.getPosition().val(1));
        itemZ->setNumber(joint.getPosition().val(2));
    }
}

void InstanceViewerWindow::feedDistancesModel(const dai::Skeleton &skeleton, QStandardItemModel& model)
{
    QMutexLocker locker(&m_modelsLock);

    // Distances Model
    for (int i=0; i<skeleton.getJointsCount(); ++i)
    {
        const SkeletonJoint& joint1 = skeleton.getJoint( (SkeletonJoint::JointType) i );

        for (int j=0; j<skeleton.getJointsCount(); ++j)
        {
            const SkeletonJoint& joint2 = skeleton.getJoint( (SkeletonJoint::JointType) j );
            float distance = 0;

            if (j < i) {
                distance = Point3f::euclideanDistance(joint1.getPosition(), joint2.getPosition());
            } else if (j > i) {
                distance = -1;
            }

            CustomItem* item = dynamic_cast<CustomItem*>(model.item(i, j));
            item->setNumber(distance);
        }
    }

    m_distances_table_view.resizeColumnsToContents();
}

void InstanceViewerWindow::feedQuaternionsModel(const dai::Skeleton &skeleton, QStandardItemModel& model)
{
    QMutexLocker locker(&m_modelsLock);

    // Quaternions Model
    for (int i=0; i<20; ++i)
    {
        const Quaternion& quaternion = skeleton.getQuaternion( (Quaternion::QuaternionType) i );
        Quaternion& lastQuaternion = m_lastQuaternions[i];
        double distance = Quaternion::getDistanceBetween(lastQuaternion, quaternion);

        if (m_frameCounter % 5 == 0)
            m_lastQuaternions[i] = quaternion;

        QStandardItem* itemTensor = model.item(i, 0);
        QStandardItem* itemX = model.item(i, 1);
        QStandardItem* itemY = model.item(i, 2);
        QStandardItem* itemZ = model.item(i, 3);
        QStandardItem* itemAngle = model.item(i, 4);

        itemTensor->setText(QString::number(quaternion.scalar()));
        itemX->setText(QString::number(quaternion.vector().x()));
        itemY->setText(QString::number(quaternion.vector().y()));
        itemZ->setText(QString::number(quaternion.vector().z()));
        itemAngle->setText(QString::number(distance));

        QBrush brush;
        brush.setStyle(Qt::SolidPattern);
        QColor color(255, 255, 255);
        color.setRgb(255, 0, 0);
        color.setAlphaF(distance);

        brush.setColor(color);
        itemTensor->setBackground(brush);
        itemX->setBackground(brush);
        itemY->setBackground(brush);
        itemZ->setBackground(brush);
        itemAngle->setBackground(brush);
    }

    m_frameCounter++;
}

float InstanceViewerWindow::colorIntensity(float x)
{
    const double b = 2.30258509299405;
    const double max = 2.39789527279837;
    // max =  1.79175946922805
    // b =  2.99573227355399
    return (log(100*(x+0.1)) - b)/max;
}

void InstanceViewerWindow::measureTime(qint64 initialTime)
{
    static qint64 total = 0;
    static qint64 counter = 0;
    static qint64 max = 0;
    static qint64 min = 99999999999999999;

    qint64 timeReceived = producerHandler()->superTimer.nsecsElapsed();
    qint64 diff = timeReceived - initialTime;
    total += diff;
    counter++;

    if (diff > max) max = diff;
    if (diff < min) min = diff;

    qDebug() << "Diff (ms)" << diff / 1000000.0f <<
                "Max (ms)" << max / 1000000.0f <<
                "Min (ms)" << min / 1000000.0f <<
                "Avg (ms)" << (total / counter) / 1000000.0f;
}

} // End Namespace
