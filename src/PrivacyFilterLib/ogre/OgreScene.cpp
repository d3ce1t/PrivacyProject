#include "OgreScene.h"
#include "ogre/CharacterController.h"
#include <RenderSystems/GL/OgreGLTexture.h>
#include <QOpenGLContext>
#include "types/SkeletonFrame.h"
#include <QDebug>

OgreScene::OgreScene()
    : m_qtContext(nullptr)
    , m_glContext(nullptr)
    , m_qtSurface(nullptr)
    , m_resources_cfg("resources.cfg")
    , m_plugins_cfg("plugins.cfg")
    , m_root(nullptr)
    , m_ogreWindow(nullptr)
    , m_camera(nullptr)
    , m_sceneManager(nullptr)
    , m_renderTarget(nullptr)
    , m_viewport(nullptr)
    , m_chara(nullptr)
    , m_initialised(false)
    , m_lastTime(0)
    , m_userId(-1)
    , m_useGivenMatrix(false)
{   
    QSurfaceFormat format;
    format.setMajorVersion(2);
    format.setMinorVersion(0);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::SingleBuffer);

    m_surface.setTitle("Ogre Scene");
    m_surface.setSurfaceType(QSurface::OpenGLSurface);
    m_surface.setFormat(format);
    m_surface.create();

    m_width = 0;
    m_height = 0;
}

OgreScene::~OgreScene()
{
    if (m_chara) {
        delete m_chara;
        m_chara = nullptr;
    }

    if (m_root) {
        activateOgreContext();
        delete m_root;
        m_root = nullptr;
        doneOgreContext();
    }
}

void OgreScene::initialise(int width, int height)
{
    m_width = width;
    m_height = height;

    createOpenGLContext();
    activateOgreContext();

    // Setup and Start up Ogre
    m_root = new Ogre::Root(m_plugins_cfg.toStdString());

    // Set OpenGL render subsystem
    Ogre::RenderSystem *renderSystem = m_root->getRenderSystemByName("OpenGL Rendering Subsystem");
    m_root->setRenderSystem(renderSystem);
    m_root->initialise(false);

    // Setup window params
    Ogre::NameValuePairList params;

#if (defined WIN32)
    HGLRC myContext = wglGetCurrentContext();
    params["externalWindowHandle"] = Ogre::StringConverter::toString((unsigned int)m_surface.winId());
    params["externalGLControl"] = "true";
    params["externalGLContext"] = Ogre::StringConverter::toString((unsigned long) myContext);
#else
    params["externalGLControl"] = "true"; // Linux GL Renderer
    params["currentGLContext"] = "true"; // Linux GL Renderer
#endif

    m_ogreWindow = m_root->createRenderWindow("", width, height, false, &params); 
    m_sceneManager = m_root->createSceneManager(Ogre::ST_GENERIC, "mySceneManager");

    updateFBO(width, height);
    createCamera();
    updateViewport();
    setupResources();

    // Create scene, cameras and others
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
    createScene();

    doneOgreContext();

    m_timer.start();
    m_initialised = true;
}

void OgreScene::resize(int width, int height)
{
    m_width = width;
    m_height = height;
    activateOgreContext();
    m_ogreWindow->resize(width, height);

    if (m_useGivenMatrix) {
        m_camera->setCustomProjectionMatrix(true, m_matrix);
    } else {
        m_camera->setOrthoWindow(m_width, m_height);
    }
    updateFBO(width, height);
    updateViewport();
    doneOgreContext();
}

void OgreScene::prepareData(const dai::QHashDataFrames frames)
{
    qint64 time_ms = m_timer.elapsed();

    if (!m_initialised || time_ms == m_lastTime)
        return;

    // Match skeleton with the character
    if (frames.contains(dai::DataFrame::Skeleton))
    {
        shared_ptr<dai::SkeletonFrame> skeletonFrame = static_pointer_cast<dai::SkeletonFrame>( frames.value(dai::DataFrame::Skeleton) );
        int userId = skeletonFrame->getAllUsersId().isEmpty() ? 0 : skeletonFrame->getAllUsersId().at(0);

        if (userId > 0 && m_userId == -1) {
            // New user
            m_userId = userId;
            dai::SkeletonPtr skeleton = skeletonFrame->getSkeleton(userId);
            m_chara->setSkeleton(skeleton);
            m_chara->newUser(userId);
        }
        else if (userId > 0 && m_userId == userId) {
            // Same user
            shared_ptr<dai::Skeleton> skeleton = skeletonFrame->getSkeleton(userId);
            m_chara->setSkeleton(skeleton);
            Ogre::Real deltaTime = (time_ms - m_lastTime) / 1000.0f;
            m_chara->addTime(deltaTime);
            m_lastTime = time_ms;
        }
        else if (userId > 0) {
            qDebug() << "No debería llegar";
        }
        else if (userId == 0 && m_userId > 0) {
            m_chara->setSkeleton(nullptr);
            m_chara->lostUser(m_userId);
            m_userId = -1;
        }
    }
}

void OgreScene::render()
{
    activateOgreContext();

    if (m_ogreWindow && m_renderTarget) {
        m_renderTarget->update(true);
    }

    doneOgreContext();
}

void OgreScene::enableFilter(bool flag)
{
    if (m_chara) {
        m_chara->setVisible(flag);
    }
}

uint OgreScene::texture() const
{
    return m_nativeTextureId;
}

void OgreScene::setMatrix(const QMatrix4x4& matrix)
{
    m_matrix[0][0] = matrix(0,0);
    m_matrix[0][1] = matrix(0,1);
    m_matrix[0][2] = matrix(0,2);
    m_matrix[0][3] = matrix(0,3);

    m_matrix[1][0] = matrix(1,0);
    m_matrix[1][1] = matrix(1,1);
    m_matrix[1][2] = matrix(1,2);
    m_matrix[1][3] = matrix(1,3);

    m_matrix[2][0] = matrix(2,0);
    m_matrix[2][1] = matrix(2,1);
    m_matrix[2][2] = matrix(2,2);
    m_matrix[2][3] = matrix(2,3);

    m_matrix[3][0] = matrix(3,0);
    m_matrix[3][1] = matrix(3,1);
    m_matrix[3][2] = matrix(3,2);
    m_matrix[3][3] = matrix(3,3);

    m_useGivenMatrix = true;
}

void OgreScene::setupResources()
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(m_resources_cfg.toStdString());

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;

            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }

    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void OgreScene::createCamera()
{
    m_camera = m_sceneManager->createCamera("PlayerCam");

    if (m_useGivenMatrix) {
        m_camera->setCustomProjectionMatrix(true, m_matrix);
    } else {
        m_camera->setNearClipDistance(0.1f);
        m_camera->setOrthoWindow(m_width, m_height);
    }

    m_camera->setPosition(Ogre::Vector3(0, 0, 0));
    m_camera->lookAt(0, 0, 0);
    Ogre::SceneNode* m_node = Ogre::Root::getSingleton().getSceneManager("mySceneManager")->getRootSceneNode()->createChildSceneNode();
    m_node->attachObject(m_camera);
}

void OgreScene::updateFBO(int width, int height)
{
    // First create Ogre RTT texture
    Ogre::TexturePtr rttTexture = Ogre::TextureManager::getSingleton().createManual("RttTex",
                                                                     Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                                     Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_R8G8B8A8,
                                                                     Ogre::TU_RENDERTARGET, 0, false, 4);

    Ogre::GLTexture* nativeTexture = static_cast<Ogre::GLTexture *>(rttTexture.get());
    m_nativeTextureId = nativeTexture->getGLID();
    m_renderTarget = rttTexture->getBuffer()->getRenderTarget();
}

void OgreScene::updateViewport()
{
    m_viewport = m_renderTarget->addViewport(m_camera);
    m_viewport->setDepthClear(1.0f);
    m_viewport->setBackgroundColour(Ogre::ColourValue(0.0f, 0.0f, 0.0f, 0.0f));
    m_viewport->setClearEveryFrame(true);
    m_viewport->setOverlaysEnabled(false);
}

void OgreScene::createScene()
{
    // set shadow properties
    m_sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);
    m_sceneManager->setShadowColour(Ogre::ColourValue(0.5, 0.5, 0.5));
    m_sceneManager->setShadowTextureSize(1024);
    m_sceneManager->setShadowTextureCount(1);

    // use a small amount of ambient lighting
    //m_sceneManager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.3));

    // add a bright light above the scene
    Ogre::Light* light = m_sceneManager->createLight();
    light->setType(Ogre::Light::LT_POINT);
    light->setPosition(0, 15, 10);
    light->setSpecularColour(Ogre::ColourValue::White);

    // Create our character controller
    m_chara = new CharacterController(m_camera);
}

void OgreScene::convertDepthToRealWorld(int x, int y, float distance, float &outX, float &outY) const
{
    const double fd_x = 1.0 / 5.9421434211923247e+02;
    const double fd_y = 1.0 / 5.9104053696870778e+02;
    const double cd_x = 0.5 * 640;
    const double cd_y = 0.5 * 480;
    outX = (x - cd_x) * distance * fd_x;
    outY = (y - cd_y) * distance * fd_y;
}

void OgreScene::createOpenGLContext()
{
    m_qtContext = QOpenGLContext::currentContext();
    m_glContext = new QOpenGLContext;
    m_glContext->setFormat(m_surface.format());

    if (m_qtContext) {
        m_qtSurface = m_qtContext->surface();
        m_glContext->setShareContext(m_qtContext);
    } else {
        qDebug() << "WARNING: Could not create a shared context!";
    }

    if (!m_glContext->create()) {
        qDebug() << "Could not create the OpenGL context";
        throw 1;
    }
}

void OgreScene::activateOgreContext()
{
    if (m_qtContext) {
        m_qtContext->doneCurrent();
    }

    m_glContext->makeCurrent(&m_surface);
}

void OgreScene::doneOgreContext()
{
    m_glContext->doneCurrent();

    if (m_qtContext) {
        m_qtContext->makeCurrent(m_qtSurface);
    }
}
