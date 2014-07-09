#include "OgreScene.h"
#include <OgreStringConverter.h>

OgreScene::OgreScene()
    : m_resources_cfg("resources.cfg")
    , m_plugins_cfg("plugins.cfg")
    , m_glContext(nullptr)
    , m_gles(nullptr)
    , m_root(nullptr)
    , m_ogreWindow(nullptr)
    , m_camera(nullptr)
    , m_sceneManager(nullptr)
    , m_viewport(nullptr)
    , m_chara(nullptr)
    , m_lastTime(0)
    , m_userId(-1)
    , m_pointCloud(nullptr)
    , m_pDepthData(nullptr)
    , m_pColorData(nullptr)
    , m_numPoints(640*480)
    , m_initialised(false)
{
    QSurfaceFormat format;
    format.setMajorVersion(2);
    format.setMinorVersion(0);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::SingleBuffer);

    m_surface.setSurfaceType(QSurface::OpenGLSurface);
    m_surface.setFormat(format);
    m_surface.create();
    m_surface.resize(640, 480);
    m_surface.show();
}

OgreScene::~OgreScene()
{
    if (m_pDepthData) {
        delete[] m_pDepthData;
        m_pDepthData = nullptr;
    }

    if (m_pColorData) {
        delete[] m_pColorData;
        m_pColorData = nullptr;
    }

    if (m_chara) {
        delete m_chara;
        m_chara = nullptr;
    }

    if (m_pointCloud) {
        delete m_pointCloud;
        m_pointCloud = nullptr;
    }

    /*Ver después, porque puede que lo tenga que ejecutar desde el contexto OGL
     * if (m_root) {
        delete m_root;
        m_root = nullptr;
    }*/
}

void OgreScene::initialise()
{
    createOpenGLContext();
    activateOgreContext();

    // Setup and Start up Ogre
    m_root = new Ogre::Root(m_plugins_cfg);

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

    m_ogreWindow = m_root->createRenderWindow("", 640, 480, false, &params);
    m_sceneManager = m_root->createSceneManager(Ogre::ST_GENERIC, "mySceneManager");

    // First create Ogre RTT texture
    //int samples = 4;
    /*m_rttTexture = Ogre::TextureManager::getSingleton().createManual("RttTex",
                                                                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                                    Ogre::TEX_TYPE_2D,
                                                                    640,
                                                                    480,
                                                                    0,
                                                                    Ogre::PF_R8G8B8A8,
                                                                    Ogre::TU_RENDERTARGET, 0, false,
                                                                    samples);

    m_renderTarget = m_rttTexture->getBuffer()->getRenderTarget();*/

    createCamera();

    m_viewport = m_ogreWindow->addViewport(m_camera);
    m_viewport->setDepthClear(1.0f);
    m_viewport->setBackgroundColour(Ogre::ColourValue(0.0f, 0.0f, 0.0f, 0.0f));
    m_viewport->setClearEveryFrame(true);
    m_viewport->setOverlaysEnabled(false);

    setupResources();

    // Create scene, cameras and others
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
    createScene();

    m_timer.start();

    doneOgreContext();
    m_initialised = true;
}

void OgreScene::setupResources()
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(m_resources_cfg);

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
    m_camera->setNearClipDistance(0.1f);
    m_camera->setAspectRatio(4/3);
    m_camera->setFOVy(Ogre::Degree(45));
    //m_camera->setPosition(Vector3(120, 0, 0));
    m_camera->setPosition(Ogre::Vector3(0, 0, 0));
    m_camera->lookAt(0, 0, 0);

    Ogre::SceneNode* m_node = Ogre::Root::getSingleton().getSceneManager("mySceneManager")->getRootSceneNode()->createChildSceneNode();
    m_node->attachObject(m_camera);
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

    // Create a Point Cloud Mesh
    //createPointCloud();

    // Create our character controller
    m_chara = new SinbadCharacterController(m_camera);
    m_chara->setVisible(true);
}

void OgreScene::createPointCloud()
{
    // Init point cloud mesh and required data
    m_pDepthData = new float[m_numPoints*3];
    m_pColorData = new float[m_numPoints*3];
    m_pointCloud = new dai::OgrePointCloud("PointCloud", "General", m_numPoints);
    m_pointCloud->initialise();

    // Create a Point Cloud entity
    Ogre::Entity *entity = m_sceneManager->createEntity("PointCloud", "PointCloud", "General");
    entity->setMaterialName("PointCloud");
    Ogre::SceneNode *node = m_sceneManager->getRootSceneNode()->createChildSceneNode(Ogre::Vector3::UNIT_Y);
    node->scale(23, 23, 23);
    node->setPosition(0, 0, 0);
    node->attachObject(entity);
}

void OgreScene::newFrames(const dai::QHashDataFrames dataFrames)
{
    if (!m_initialised) {
        initialise();
    }

    // Copy frames (1 ms)
    dai::QHashDataFrames copyFrames;

    foreach (dai::DataFrame::FrameType key, dataFrames.keys()) {
        shared_ptr<dai::DataFrame> frame = dataFrames.value(key);
        copyFrames.insert(key, frame->clone());
    }

    // Check if the frames has been copied correctly
    if (!hasExpired())
    {
        // Do task
        this->prepareData(copyFrames);
        this->render();
    }
    else {
        qDebug() << "Frame Read but Not Valid";
    }
}

void OgreScene::prepareData(const dai::QHashDataFrames frames)
{
    qint64 time_ms = m_timer.elapsed();

    if (!m_initialised || time_ms == m_lastTime)
        return;

    // Load Depth
    if (m_pointCloud && frames.contains(dai::DataFrame::Depth)) {
        shared_ptr<dai::DepthFrame> depthFrame = static_pointer_cast<dai::DepthFrame>(frames.value(dai::DataFrame::Depth));
        loadDepthData(depthFrame);
    }

    // Load Color
    if (m_pointCloud && frames.contains(dai::DataFrame::Color)) {
        shared_ptr<dai::ColorFrame> colorFrame = static_pointer_cast<dai::ColorFrame>(frames.value(dai::DataFrame::Color));
        loadColorData(colorFrame);
    }

    // Match skeleton with the character
    if (frames.contains(dai::DataFrame::Skeleton))
    {
        shared_ptr<dai::SkeletonFrame> skeletonFrame = static_pointer_cast<dai::SkeletonFrame>( frames.value(dai::DataFrame::Skeleton) );
        int userId = skeletonFrame->getAllUsersId().isEmpty() ? 0 : skeletonFrame->getAllUsersId().at(0);

        if (userId > 0 && m_userId == -1) {
            // New user
            m_userId = userId;
            shared_ptr<dai::Skeleton> skeleton = skeletonFrame->getSkeleton(userId);
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

    if (m_pointCloud) {
        QReadLocker locker(&m_lock);
        m_pointCloud->updateVertexPositions(m_pDepthData, m_numPoints);
        m_pointCloud->updateVertexColours(m_pColorData, 640*480);
        //m_ogreEngine->doneOgreContext();
    }

    if (m_ogreWindow) {
        m_ogreWindow->update(true);
    }

    doneOgreContext();
}

void OgreScene::loadDepthData(shared_ptr<dai::DepthFrame> depthFrame)
{
    QWriteLocker locker(&m_lock);
    float* pV = m_pDepthData;
    float* pDepth = (float*) depthFrame->getDataPtr();
    pDepth = pDepth + 640*480;
    m_numPoints = 0;

    for (int i_y = 0; i_y < 480; ++i_y)
    {
        for (int i_x = 0; i_x < 640; ++i_x)
        {
            // Position
            if (*pDepth > 0) {
                int x = 640 - i_x; // flip x
                int y = i_y;
                convertDepthToRealWorld(x, y, *pDepth, pV[0], pV[1]);
                pV[2] = -(*pDepth); // meters to cm (ogre +z is out of the screen)
            } else {
                pV[0] = 0.0f;
                pV[1] = 0.0f;
                pV[2] = 0.0f;
            }

            pV+=3;
            pDepth--;
            m_numPoints++;
        }
    }
}

void OgreScene::loadColorData(shared_ptr<dai::ColorFrame> colorFrame)
{
    QWriteLocker locker(&m_lock);
    float* pColorDest = m_pColorData;
    dai::RGBColor* pColorSource = (dai::RGBColor*) colorFrame->getDataPtr();
    pColorSource = pColorSource + 640*480;
    m_numPoints = 0;

    for (int i_y = 0; i_y < 480; ++i_y)
    {
        for (int i_x = 0; i_x < 640; ++i_x)
        {
            // Update color
            pColorDest[0] = pColorSource->red / 255.0f;
            pColorDest[1] = pColorSource->green / 255.0f;
            pColorDest[2] = pColorSource->blue / 255.0f;

            // Move pointers
            pColorDest += 3;
            pColorSource--;
            m_numPoints++;
        }
    }
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

void OgreScene::enableFilter(bool flag)
{
    if (m_chara) {
        m_chara->setVisible(flag);
    }
}

void OgreScene::createOpenGLContext()
{
    m_glContext = new QOpenGLContext;
    m_glContext->setFormat(m_surface.format());

    if (!m_glContext->create()) {
        qDebug() << "Could not create the OpenGL context";
        throw 1;
    }

    /*m_glContext->makeCurrent(&m_surface);
    m_gles = m_glContext->functions();
    */
}

void OgreScene::activateOgreContext()
{
    //glPopAttrib();
    //glPopClientAttrib();

    //m_qtContext->functions()->glUseProgram(0);
    //m_qtContext->doneCurrent();

    m_glContext->makeCurrent(&m_surface);
}

void OgreScene::doneOgreContext()
{
    /*m_ogreContext->functions()->glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    m_ogreContext->functions()->glBindRenderbuffer(GL_RENDERBUFFER, 0);
    m_ogreContext->functions()->glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

    // unbind all possible remaining buffers; just to be on safe side
    m_ogreContext->functions()->glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_COPY_READ_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
//    m_ogreContext->functions()->glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
//    m_ogreContext->functions()->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_TEXTURE_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_UNIFORM_BUFFER, 0);
*/
    m_glContext->doneCurrent();

    //m_qtContext->makeCurrent(m_quickWindow);
    //glPushAttrib(GL_ALL_ATTRIB_BITS);
    //glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
}
