/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#include "OgreCharacter.h"

#include "ogreitem.h"
#include "ogreengine.h"

#include <QCoreApplication>
#include <QtQml/QQmlContext>
#include <QDir>

OgreCharacter::OgreCharacter(QWindow *parent) :
    QQuickView(parent)
  , m_cameraObject(0)
  , m_ogreEngine(0)
  , m_root(0)
  , m_camera(0)
  //, m_viewPort(0)
  , m_sceneManager(0)
  , mChara(0)
  , m_lastTime(0)
{
    // start Ogre once we are in the rendering thread (Ogre must live in the rendering thread)
    connect(this, &OgreCharacter::beforeRendering, this, &OgreCharacter::initializeOgre, Qt::DirectConnection);
    connect(this, &OgreCharacter::ogreInitialized, this, &OgreCharacter::addContent);
}

OgreCharacter::~OgreCharacter()
{
    destroyScene();

    if (m_sceneManager) {
        m_root->destroySceneManager(m_sceneManager);
    }
}

void OgreCharacter::initializeOgre()
{
    // we only want to initialize once
    disconnect(this, &OgreCharacter::beforeRendering, this, &OgreCharacter::initializeOgre);

    // start up Ogrez
    m_ogreEngine = new OgreEngine;
    m_ogreEngine->startEngine(this);
    m_root = m_ogreEngine->root();

    // set up Ogre scene
    m_sceneManager = m_root->createSceneManager(Ogre::ST_GENERIC, "mySceneManager");

    //
    // Setup
    //
    createCamera();
    createViewports();

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    createScene();

    emit(ogreInitialized());
}

void OgreCharacter::createCamera(void)
{
    m_camera = m_sceneManager->createCamera("PlayerCam");
    m_camera->setNearClipDistance(5);
    //m_camera->setFarClipDistance(99999);
    m_camera->setAspectRatio(Ogre::Real(width()) / Ogre::Real(height()));
    m_camera->setAutoTracking(true, m_sceneManager->getRootSceneNode());
    m_cameraObject = new CameraNodeObject;
    m_cameraObject->setCamera(m_camera);
}

void OgreCharacter::createViewports(void)
{
    // Create one viewport, entire window
    m_viewPort = m_ogreEngine->renderWindow()->addViewport(m_camera);
    m_viewPort->setBackgroundColour(Ogre::ColourValue(1.0,1.0,1.0));
}

void OgreCharacter::createScene(void)
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
    mChara = new SinbadCharacterController(m_camera);

    setupDepthMaterial();
    /*mDepthPanel = Ogre::OverlayManager::getSingleton().createOverlayElement("Panel","DepthPanel");
    mDepthPanel->setMaterialName("DepthTextureMaterial");
    mDepthPanel->setMetricsMode(Ogre::GMM_RELATIVE);
    mDepthPanel->setWidth(0.25);
    mDepthPanel->setHeight(0.25*480/640);
    mDepthPanel->setHorizontalAlignment(GHA_RIGHT);
    mDepthPanel->setVerticalAlignment(GVA_BOTTOM);
    mDepthPanel->setLeft(-mDepthPanel->getWidth());
    mDepthPanel->setTop(-mDepthPanel->getHeight());

    mDepthPanel->show();*/
}

void OgreCharacter::destroyScene(void)
{
    // clean up character controller and the floor mesh
    if (mChara) delete mChara;
    MeshManager::getSingleton().remove("floor");
}

void OgreCharacter::addContent()
{
    // expose objects as QML globals
    rootContext()->setContextProperty("Window", this);
    rootContext()->setContextProperty("Camera", m_cameraObject);
    rootContext()->setContextProperty("OgreEngine", m_ogreEngine);

    // load the QML scene
    setResizeMode(QQuickView::SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/example.qml"));

   connect(m_ogreEngine, &OgreEngine::beforeRendering, this, &OgreCharacter::addTime, Qt::DirectConnection);
}

void OgreCharacter::addTime(qint64 time_ms)
{        
    if (time_ms == m_lastTime)
        return;

    Real deltaTime = (time_ms - m_lastTime) / 1000.0f;
    mChara->addTime(deltaTime);
    m_lastTime = time_ms;
}

void OgreCharacter::setupDepthMaterial()
{
    // Create the texture
    TextureManager::getSingleton().createManual(
            "MyDepthTexture", // name
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            TEX_TYPE_2D,      // type
            640, 480,         // width & height
            0,                // number of mipmaps
            PF_BYTE_BGRA,     // pixel format
            TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);


    // Create a material using the texture
    MaterialPtr material = MaterialManager::getSingleton().create(
            "DepthTextureMaterial", // name
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    material->getTechnique(0)->getPass(0)->createTextureUnitState("MyDepthTexture");
    material->getTechnique(0)->getPass(0)->setSceneBlending(SBT_TRANSPARENT_ALPHA);
}
