#include "OgreScene.h"

OgreScene::OgreScene()
    : QObject()
    , m_root(nullptr)
    , m_camera(nullptr)
    , m_sceneManager(nullptr)
    , m_chara(nullptr)
    , m_lastTime(0)
    , m_userId(-1)
{
    m_ogreEngine = new OgreEngine;
    m_cameraObject = new CameraNodeObject;
}

CameraNodeObject* OgreScene::cameraNode()
{
    return m_cameraObject;
}

OgreEngine* OgreScene::engine()
{
    return m_ogreEngine;
}

void OgreScene::initialiseOgre(QQuickWindow* quickWindow)
{
    // Setup and Start up Ogre
    m_quickWindow = quickWindow;
    m_ogreEngine->startEngine(quickWindow);

    m_ogreEngine->activateOgreContext();

    m_root = m_ogreEngine->root();
    m_sceneManager = m_root->createSceneManager(Ogre::ST_GENERIC, "mySceneManager");

    createCamera();

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    createScene();

    m_ogreEngine->doneOgreContext();
    m_timer.start();
}

void OgreScene::createCamera(void)
{
    m_camera = m_sceneManager->createCamera("PlayerCam");
    m_camera->setNearClipDistance(5);
    m_camera->setAspectRatio(Ogre::Real(m_quickWindow->width()) / Ogre::Real(m_quickWindow->height()));
    m_camera->setAutoTracking(true, m_sceneManager->getRootSceneNode());
    m_cameraObject->setCamera(m_camera);
}

void OgreScene::createScene(void)
{
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
    //Ogre::MeshManager::getSingleton().createPlane("floor", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    //    Ogre::Plane(Ogre::Vector3::UNIT_Y, 0),100, 100, 10, 10, true, 1, 10, 10, Ogre::Vector3::UNIT_Z);

    // create a floor entity, give it a material, and place it at the origin
    //Ogre::Entity* floor = m_sceneManager->createEntity("Floor", "floor");
    //floor->setMaterialName("Examples/Rockwall");
    //floor->setCastShadows(false);
    //m_sceneManager->getRootSceneNode()->attachObject(floor);

    // create our character controller
    m_chara = new SinbadCharacterController(m_camera);
}

void OgreScene::destroyScene(void)
{
    // clean up character controller and the floor mesh
    if (m_chara) delete m_chara;
    //Ogre::MeshManager::getSingleton().remove("floor");
}

void OgreScene::onNewFrame(const QHash<dai::DataFrame::FrameType, shared_ptr<dai::DataFrame>>& frames)
{
    if (!frames.contains(dai::DataFrame::Skeleton))
        return;

    qint64 time_ms = m_timer.elapsed();

    if (time_ms == m_lastTime)
        return;

    shared_ptr<dai::SkeletonFrame> skeletonFrame = static_pointer_cast<dai::SkeletonFrame>( frames.value(dai::DataFrame::Skeleton) );
    int userId = skeletonFrame->getAllUsersId().isEmpty() ? 0 : skeletonFrame->getAllUsersId().at(0);

    if (userId > 0 && m_userId == -1) {
        // New user
        qDebug() << "New User";
        m_userId = userId;
        shared_ptr<dai::Skeleton> skeleton = skeletonFrame->getSkeleton(userId);
        m_chara->setSkeleton(skeleton);
        m_chara->newUser(userId);
    }
    else if (userId > 0 && m_userId == userId) {
        // Same user
        //qDebug() << "Same user";
        shared_ptr<dai::Skeleton> skeleton = skeletonFrame->getSkeleton(userId);
        m_chara->setSkeleton(skeleton);
        Real deltaTime = (time_ms - m_lastTime) / 1000.0f;
        m_chara->addTime(deltaTime);
        m_lastTime = time_ms;
    }
    else if (userId > 0) {
        qDebug() << "No debería llegar";
    }
    else if (userId == 0 && m_userId > 0) {
        qDebug() << "User Lost";
        m_chara->setSkeleton(nullptr);
        m_chara->lostUser(m_userId);
        m_userId = -1;
    }
}
