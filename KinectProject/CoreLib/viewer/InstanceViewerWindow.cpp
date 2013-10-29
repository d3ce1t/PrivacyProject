#include "InstanceViewerWindow.h"
#include "playback/PlaybackControl.h"
#include "InstanceWidgetItem.h"
#include "dataset/Dataset.h"
#include "dataset/InstanceInfo.h"
#include "types/UserFrame.h"
#include <QQmlContext>
#include "CustomItem.h"

namespace dai {

InstanceViewerWindow::InstanceViewerWindow()
{ 
    // Viewer setup
    m_fps = 0;

    // QML Setup
    qmlRegisterType<InstanceViewer>("InstanceViewer", 1, 0, "InstanceViewer");
    qmlRegisterUncreatableType<QMLEnumsWrapper>("edu.dai.kinect", 1, 0, "ColorFilter", "This exports SomeState enums to QML");
    qRegisterMetaType<QHashDataFrames>("QHashDataFrames");
    qRegisterMetaType<QList<shared_ptr<BaseInstance>>>("QList<shared_ptr<BaseInstance>>");
    qRegisterMetaType<shared_ptr<SkeletonFrame>>("shared_ptr<SkeletonFrame>");

    m_engine.rootContext()->setContextProperty("viewerWindow", (QObject *) this);
    m_engine.load(QUrl("qrc:///qml/qml/main.qml"));
    QObject *topLevel = m_engine.rootObjects().value(0);
    m_window = qobject_cast<QQuickWindow *>(topLevel);

    if ( !m_window ) {
        qWarning("Error: Your root item has to be a Window.");
        return;
    }

    m_viewer = m_window->findChild<InstanceViewer*>("viewer");

    if (!m_viewer) {
        qWarning("Error: Viewer not found.");
        return;
    }

    // Windows setup
    connect(m_window, SIGNAL(closing(QQuickCloseEvent*)), this, SLOT(deleteLater()));
    connect(m_viewer, SIGNAL(frameRendered()), this, SLOT(completeAsyncTask()));
    setTitle("Instance Viewer");
}

InstanceViewerWindow::~InstanceViewerWindow()
{
    qDebug() << "InstanceViewerWindow::~InstanceViewerWindow()";
    m_viewer = nullptr;
    m_window = nullptr;

    // Close windows and clear models
    m_joints_table_view.close();
    m_joints_model.clear();

    m_distances_table_view.close();
    m_distances_model.clear();

    m_quaternions_table_view.close();
    m_quaternions_model.clear();
}

void InstanceViewerWindow::processListItem(QListWidget* widget)
{
    if (widget == nullptr)
        return;

    InstanceWidgetItem* instanceItem = (InstanceWidgetItem*) widget->selectedItems().at(0);
    InstanceInfo& info = instanceItem->getInfo();
    const Dataset& dataset = info.parent().dataset();
    shared_ptr<BaseInstance> instance = dataset.getInstance(info);

    if (instance) {
        playback()->removeListener(this, instance->getType());
        playback()->addInstance(instance);
        playback()->addListener(this, instance);
        playback()->play(true);
        setTitle("Instance Viewer (" + instance->getTitle() + ")");
    }
}

// called from Notifier thread
void InstanceViewerWindow::onNewFrame(const QHash<DataFrame::FrameType, shared_ptr<DataFrame>>& dataFrames)
{
    // Sent to viewer (execute the method in the thread it belongs to)
    QMetaObject::invokeMethod(m_viewer, "onNewFrame",
                                  Qt::AutoConnection,
                                  Q_ARG(QHashDataFrames, dataFrames));

    // Feed skeleton data models
    if (dataFrames.contains(DataFrame::Skeleton)) {
        shared_ptr<SkeletonFrame> skeleton = static_pointer_cast<SkeletonFrame>( dataFrames.value(DataFrame::Skeleton) );
        QMetaObject::invokeMethod(this, "feedDataModels",
                                      Qt::AutoConnection,
                                      Q_ARG(shared_ptr<SkeletonFrame>, skeleton));
    }

    // ¿Why is this causing flickering?
    m_fps = playback()->getFPS();
    emit changeOfStatus();

    // Block notifier thread
    PlaybackListener::startAsyncTask();
}

void InstanceViewerWindow::completeAsyncTask()
{
    PlaybackListener::endAsyncTask();
}

float InstanceViewerWindow::getFPS() const
{
    return m_fps;
}

void InstanceViewerWindow::setTitle(const QString& title)
{
    if (m_window)
        m_window->setTitle(title);
}

void InstanceViewerWindow::show()
{
    if (m_window)
        m_window->show();
}

void InstanceViewerWindow::setMode(ViewerMode mode)
{
    m_viewer->setMode(mode);
}

const InstanceViewer* InstanceViewerWindow::viewer()
{
    return m_viewer;
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
    // Setup Joints Model
    model.setRowCount(20);
    model.setColumnCount(3);
    m_joints_table_view.setWindowTitle("Joints info");
    m_joints_table_view.setModel(&model);
    m_joints_table_view.setMinimumSize(460, 630);

    const QMetaObject &metaObject = dai::SkeletonJoint::staticMetaObject;
    int index = metaObject.indexOfEnumerator("JointType"); // watch out during refactorings
    QMetaEnum metaEnum = metaObject.enumerator(index);

#if (!defined _MSC_VER)
    QStringList list = {"pos X", "pos Y", "pos Z"};
#else
    QStringList list;
    list.append("pos X");
    list.append("pos Y");
    list.append("pos Z");

#endif
    model.setHorizontalHeaderLabels(list);
    list.clear();

    for (int i=0; i<20; ++i) {
        QString name(metaEnum.valueToKey(i));
        list << name.mid(6);
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
    // Setup Distances model
    model.setRowCount(20);
    model.setColumnCount(20);
    m_distances_table_view.setWindowTitle("Distances info");
    m_distances_table_view.setModel(&model);
    m_distances_table_view.setMinimumSize(600, 640);

    const QMetaObject &skeletonJointMetaObject = SkeletonJoint::staticMetaObject;
    int index = skeletonJointMetaObject.indexOfEnumerator("JointType"); // watch out during refactorings
    QMetaEnum metaEnum = skeletonJointMetaObject.enumerator(index);

    QStringList list;

    for (int i=0; i<20; ++i) {
        QString name(metaEnum.valueToKey(i));
        list << name.mid(6);
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
    m_frameCounter = 0;

    // Setup Quaternions Model
    model.setRowCount(20);
    model.setColumnCount(5);
    m_quaternions_table_view.setWindowTitle("Quaternions info");
    m_quaternions_table_view.setModel(&model);
    m_quaternions_table_view.setMinimumSize(510, 545);

    const QMetaObject &quaternionMetaObject = Quaternion::staticMetaObject;
    int index = quaternionMetaObject.indexOfEnumerator("QuaternionType");
    QMetaEnum metaEnum = quaternionMetaObject.enumerator(index);

#if (!defined _MSC_VER)
    QStringList list = {"Tensor", "pos X", "pos Y", "pos Z", "angle"};
#else
    QStringList list;
    list.append("Tensor");
    list.append("pos X");
    list.append("pos Y");
    list.append("pos Z");
    list.append("angle");
#endif

    model.setHorizontalHeaderLabels(list);
    list.clear();

    for (int i=0; i<20; ++i) {
        QString name(metaEnum.valueToKey(i));
        list << name.mid(11);
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
    // Joints Model
    for (int i=0; i<skeleton.getJointsCount(); ++i)
    {
        const SkeletonJoint& joint = skeleton.getJoint( (SkeletonJoint::JointType) i );

        CustomItem* itemX = dynamic_cast<CustomItem*>(model.item(i, 0));
        CustomItem* itemY = dynamic_cast<CustomItem*>(model.item(i, 1));
        CustomItem* itemZ = dynamic_cast<CustomItem*>(model.item(i, 2));

        itemX->setNumber(joint.getPosition().x());
        itemY->setNumber(joint.getPosition().y());
        itemZ->setNumber(joint.getPosition().z());
    }
}

void InstanceViewerWindow::feedDistancesModel(const dai::Skeleton &skeleton, QStandardItemModel& model)
{
    // Distances Model
    for (int i=0; i<skeleton.getJointsCount(); ++i)
    {
        const SkeletonJoint& joint1 = skeleton.getJoint( (SkeletonJoint::JointType) i );

        for (int j=0; j<skeleton.getJointsCount(); ++j)
        {
            const SkeletonJoint& joint2 = skeleton.getJoint( (SkeletonJoint::JointType) j );
            float distance = 0;

            if (j < i) {
                distance = Point3f::distance(joint1.getPosition(), joint2.getPosition(), Point3f::DISTANCE_EUCLIDEAN);
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

} // End Namespace
