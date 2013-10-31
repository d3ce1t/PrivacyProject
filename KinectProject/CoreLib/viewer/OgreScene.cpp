#include "OgreScene.h"

OgreScene::OgreScene()
    : m_root(nullptr)
    , m_camera(nullptr)
    , m_viewPort(nullptr)
    , m_sceneManager(nullptr)
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
    m_root = m_ogreEngine->root();
    m_sceneManager = m_root->createSceneManager(Ogre::ST_GENERIC, "mySceneManager");
    m_ogreEngine->startEngine(quickWindow);
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

void OgreScene::createCamera(void)
{
    m_camera = m_sceneManager->createCamera("PlayerCam");
    m_camera->setNearClipDistance(5);
    //m_camera->setFarClipDistance(99999);
    m_camera->setAspectRatio(Ogre::Real(m_quickWindow->width()) / Ogre::Real(m_quickWindow->height()));
    m_camera->setAutoTracking(true, m_sceneManager->getRootSceneNode());
    m_cameraObject->setCamera(m_camera);
}

void OgreScene::createViewports(void)
{
    // Create one viewport, entire window
    m_viewPort = m_ogreEngine->renderWindow()->addViewport(m_camera);
    m_viewPort->setBackgroundColour(Ogre::ColourValue(1.0,1.0,1.0));
}

void OgreScene::createScene(void)
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

void OgreScene::destroyScene(void)
{
    // clean up character controller and the floor mesh
    //if (mChara) delete mChara;
    Ogre::MeshManager::getSingleton().remove("floor");
}
