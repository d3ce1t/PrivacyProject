#include "InstanceViewerWindow.h"
#include "viewer/InstanceViewer.h"
#include "playback/PlaybackControl.h"
#include "InstanceWidgetItem.h"
#include "dataset/Dataset.h"
#include "dataset/InstanceInfo.h"
#include "types/UserFrame.h"
#include "filters/InvisibilityFilter.h"
#include "filters/DilateUserFilter.h"
#include "filters/BlurFilter.h"
#include <QQmlContext>
#include "CustomItem.h"

namespace dai {

InstanceViewerWindow::InstanceViewerWindow()
{ 
    // Viewer setup
    m_fps = 0;

    // QML Setup
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

    // Filters setup
    //shared_ptr<BlurFilter> blurFilter(new BlurFilter);
    //shared_ptr<InvisibilityFilter> invisibilityFilter(new InvisibilityFilter);
    shared_ptr<DilateUserFilter> dilateFilter(new DilateUserFilter);

    dilateFilter->enableFilter(true);

    // Filters are later retrieved from more recently to less recently inserted
    //m_filters.insert(DataFrame::Color, blurFilter);
    //m_filters.insert(DataFrame::Color, invisibilityFilter);
    m_filters.insert(DataFrame::User, dilateFilter);
}

InstanceViewerWindow::~InstanceViewerWindow()
{
    qDebug() << "InstanceViewerWindow::~InstanceViewerWindow()";
    m_viewer = nullptr;
    m_window = nullptr;
    m_filters.clear();

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
    QHash<DataFrame::FrameType, shared_ptr<DataFrame>> filteredFrames;
    shared_ptr<UserFrame> userMask;

    // Get UserFrame in order to use as mask
    if (dataFrames.contains(DataFrame::User)) {
        // Apply Filter to Mask for later use
        shared_ptr<DataFrame> filteredUserMask = applyFilter( dataFrames.value(DataFrame::User) );
        userMask = static_pointer_cast<UserFrame>( filteredUserMask );
        filteredFrames.insert(DataFrame::User, userMask);
    }

    // Then apply filters to the rest of frames
    foreach (shared_ptr<DataFrame> inputFrame, dataFrames) {
        if (inputFrame->getType() == DataFrame::User) {
            continue;
        }

        shared_ptr<DataFrame> outputFrame = applyFilter(inputFrame, userMask);
        filteredFrames.insert(outputFrame->getType(), outputFrame);
    }

    // Sent to viewer
    // I want to execute method in the thread it belongs to
    QMetaObject::invokeMethod(m_viewer, "onNewFrame",
                                  Qt::AutoConnection,
                                  Q_ARG(QHashDataFrames, filteredFrames));

    // Feed skeleton data models
    if (dataFrames.contains(DataFrame::Skeleton)) {
        shared_ptr<SkeletonFrame> skeleton = static_pointer_cast<SkeletonFrame>( dataFrames.value(DataFrame::Skeleton) );
        QMetaObject::invokeMethod(this, "feedDataModels",
                                      Qt::AutoConnection,
                                      Q_ARG(shared_ptr<SkeletonFrame>, skeleton));
    }

    // ¿Why this cause flickering?
    m_fps = playback()->getFPS();
    emit changeOfStatus();

    // Block notifier thread
    PlaybackListener::startAsyncTask();
}

void InstanceViewerWindow::completeAsyncTask()
{
    PlaybackListener::endAsyncTask();
}

shared_ptr<DataFrame> InstanceViewerWindow::applyFilter(shared_ptr<DataFrame> inputFrame, shared_ptr<UserFrame> userMask) const
{    
    QList<shared_ptr<FrameFilter>> filters = m_filters.values(inputFrame->getType());

    if (filters.count() == 0)
        return inputFrame;

    // I clone the frame because I do not want to modify the frame read by the instance
    shared_ptr<DataFrame> outputFrame = inputFrame->clone();

    foreach (shared_ptr<FrameFilter> frameFilter, filters)
    {
        frameFilter->setMask(userMask);
        frameFilter->applyFilter(outputFrame);
        frameFilter->setMask(nullptr); // Hack
    }

    return outputFrame;
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

/*void InstanceViewerWindow::enableInvisibilityFilter()
{
    shared_ptr<FrameFilter> filter = m_filters.values(DataFrame::Color).at(0);

    if (m_activeFilterArray[DataFrame::Color])
        m_activeFilterArray[DataFrame::Color]->enableFilter(false);

    m_activeFilterArray[DataFrame::Color] = filter;
    filter->enableFilter(true);
}

void InstanceViewerWindow::enableBlurFilter()
{
    shared_ptr<FrameFilter> filter = m_filters.values(DataFrame::Color).at(1);

    if (m_activeFilterArray[DataFrame::Color])
        m_activeFilterArray[DataFrame::Color]->enableFilter(false);

    m_activeFilterArray[DataFrame::Color] = filter;
    filter->enableFilter(true);
}

void InstanceViewerWindow::disableColorFilter()
{
    if (m_activeFilterArray[DataFrame::Color]) {
        m_activeFilterArray[DataFrame::Color]->enableFilter(false);
        m_activeFilterArray[DataFrame::Color] = nullptr;
    }
}*/

void InstanceViewerWindow::show()
{
    if (m_window)
        m_window->show();
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

    QStringList list = {"pos X", "pos Y", "pos Z"};
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

    QStringList list = {"Tensor", "pos X", "pos Y", "pos Z", "angle"};
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
