#include "InstanceViewerWindow.h"
#include "playback/PlaybackControl.h"
#include "InstanceWidgetItem.h"
#include "dataset/Dataset.h"
#include "dataset/InstanceInfo.h"
#include "types/UserFrame.h"
#include <QQmlContext>
#include "CustomItem.h"
#include <QMetaEnum>

namespace dai {

InstanceViewerWindow::InstanceViewerWindow(ViewerMode mode)
    : m_fps(0)
    , m_cameraObject(nullptr)
    , m_ogreEngine(nullptr)
    , m_root(nullptr)
    , m_camera(nullptr)
    , m_viewPort(nullptr)
    , m_sceneManager(nullptr)
    , m_viewerEngine(nullptr)
    , m_viewerMode(mode)
    , m_quickWindow(nullptr)
    , m_frameCounter(0)
{
    m_ogreEngine = new OgreEngine;
    m_cameraObject = new CameraNodeObject;
    m_viewerEngine = new ViewerEngine;
    m_viewerEngine->setMode(mode);

    // exposte objects as QML globals
    m_qmlEngine.rootContext()->setContextProperty("Window", this);
    m_qmlEngine.rootContext()->setContextProperty("Camera", m_cameraObject);
    m_qmlEngine.rootContext()->setContextProperty("OgreEngine", m_ogreEngine);
    m_qmlEngine.rootContext()->setContextProperty("ViewerEngine", m_viewerEngine);

    // Load QML app
    m_qmlEngine.load(QUrl("qrc:///qml/qml/main.qml"));

    // Get Window
    QObject *topLevel = m_qmlEngine.rootObjects().value(0);
    m_quickWindow = qobject_cast<QQuickWindow *>(topLevel);

    if ( !m_quickWindow ) {
        qWarning("Error: Your root item has to be a Window.");
        return;
    }

    m_quickWindow->setTitle("Instance Viewer");

    // start Ogre once we are in the rendering thread (Ogre must live in the rendering thread)
    connect(m_quickWindow, &QQuickWindow::beforeSynchronizing, this, &InstanceViewerWindow::initialiseOgre, Qt::DirectConnection);
    //connect(this, &ExampleApp::ogreInitialized, this, &ExampleApp::addContent);

    // Windows setup
    connect(m_quickWindow, SIGNAL(closing(QQuickCloseEvent*)), this, SLOT(deleteLater()));
    connect(m_viewerEngine, &ViewerEngine::frameRendered, this, &InstanceViewerWindow::completeAsyncTask);
}

InstanceViewerWindow::~InstanceViewerWindow()
{
    qDebug() << "InstanceViewerWindow::~InstanceViewerWindow()";
    m_viewerEngine = nullptr;

    // Close windows and clear models
    m_joints_table_view.close();
    m_joints_model.clear();

    m_distances_table_view.close();
    m_distances_model.clear();

    m_quaternions_table_view.close();
    m_quaternions_model.clear();
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

void InstanceViewerWindow::initialiseOgre()
{
    // we only want to initialize once
    disconnect(m_quickWindow, &QQuickWindow::beforeSynchronizing, this, &InstanceViewerWindow::initialiseOgre);

    m_viewerEngine->startEngine(m_quickWindow);

    // Setup and Start up Ogre
    m_root = m_ogreEngine->root();
    m_sceneManager = m_root->createSceneManager(Ogre::ST_GENERIC, "mySceneManager");
    m_ogreEngine->startEngine(m_quickWindow);
    m_ogreEngine->setupResources();

    //
    // Setup
    //
    createCamera();
    createViewports();

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    createScene();
}

void InstanceViewerWindow::createCamera(void)
{
    m_camera = m_sceneManager->createCamera("PlayerCam");
    m_camera->setNearClipDistance(5);
    //m_camera->setFarClipDistance(99999);
    m_camera->setAspectRatio(Ogre::Real(m_quickWindow->width()) / Ogre::Real(m_quickWindow->height()));
    m_camera->setAutoTracking(true, m_sceneManager->getRootSceneNode());
    m_cameraObject->setCamera(m_camera);
}

void InstanceViewerWindow::createViewports(void)
{
    // Create one viewport, entire window
    m_viewPort = m_ogreEngine->renderWindow()->addViewport(m_camera);
    m_viewPort->setBackgroundColour(Ogre::ColourValue(1.0,1.0,1.0));
}

void InstanceViewerWindow::createScene(void)
{
    // Resources with textures must be loaded within Ogre's GL context
    m_ogreEngine->activateOgreContext();

    // set background and some fog
    m_viewPort->setBackgroundColour(Ogre::ColourValue(1.0f, 1.0f, 0.8f));
    m_sceneManager->setFog(Ogre::FOG_LINEAR, Ogre::ColourValue(1.0f, 1.0f, 0.8f), 0,15, 100);

    // set shadow properties
    m_sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);
    m_sceneManager->setShadowColour(Ogre::ColourValue(0.5, 0.5, 0.5));
    m_sceneManager->setShadowTextureSize(1024);
    m_sceneManager->setShadowTextureCount(1);

    // use a small amount of ambient lighting
    m_sceneManager->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.3));

    // add a bright light above the scene
    Ogre::Light* light = m_sceneManager->createLight();
    light->setType(Ogre::Light::LT_POINT);
    light->setPosition(-10, 40, 20);
    light->setSpecularColour(Ogre::ColourValue::White);

    // create a floor mesh resource
    Ogre::MeshManager::getSingleton().createPlane("floor", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::Plane(Ogre::Vector3::UNIT_Y, 0),100, 100, 10, 10, true, 1, 10, 10, Ogre::Vector3::UNIT_Z);

    // create a floor entity, give it a material, and place it at the origin
    Ogre::Entity* floor = m_sceneManager->createEntity("Floor", "floor");
    floor->setMaterialName("Examples/Rockwall");
    floor->setCastShadows(false);
    m_sceneManager->getRootSceneNode()->attachObject(floor);

    m_ogreEngine->doneOgreContext();

    // create our character controller
    //mChara = new SinbadCharacterController(m_camera);
}

void InstanceViewerWindow::destroyScene(void)
{
    // clean up character controller and the floor mesh
    //if (mChara) delete mChara;
    Ogre::MeshManager::getSingleton().remove("floor");
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
    QMetaObject::invokeMethod(m_viewerEngine, "onNewFrame",
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
