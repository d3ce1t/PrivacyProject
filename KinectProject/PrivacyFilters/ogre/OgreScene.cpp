#include "OgreScene.h"

OgreScene::OgreScene()
    : m_root(nullptr)
    , m_camera(nullptr)
    , m_sceneManager(nullptr)
    , m_chara(nullptr)
    , m_lastTime(0)
    , m_userId(-1)
    , m_pointCloud(nullptr)
    , m_pDepthData(nullptr)
    , m_pColorData(nullptr)
    , m_numPoints(640*480)
    , m_initialised(false)
{
    m_ogreEngine = new OgreEngine;
    m_cameraObject = new CameraNodeObject;
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

    // Create scene, cameras and others
    m_ogreEngine->activateOgreContext();
    m_root = m_ogreEngine->root();
    m_sceneManager = m_root->createSceneManager(Ogre::ST_GENERIC, "mySceneManager");
    createCamera();
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
    createScene();
    m_ogreEngine->doneOgreContext();

    m_timer.start();
    m_initialised = true;
}

void OgreScene::createCamera(void)
{
    m_camera = m_sceneManager->createCamera("PlayerCam");
    m_camera->setNearClipDistance(0.1f);
    m_camera->setAspectRatio(4/3);
    m_camera->setFOVy(Ogre::Degree(45));
    //m_camera->setPosition(Vector3(120, 0, 0));
    m_camera->setPosition(Ogre::Vector3(0, 0, 0));
    m_camera->lookAt(0, 0, 0);
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

void OgreScene::destroyScene(void)
{
    if (m_chara) {
        delete m_chara;
        m_chara = nullptr;
    }

    if (m_pointCloud) {
        delete m_pointCloud;
        m_pointCloud = nullptr;
    }
}

void OgreScene::newFrames(const dai::QHashDataFrames frames, const qint64 frameId)
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

    // Load Skeleton
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

void OgreScene::renderOgre()
{
    if (m_pointCloud)
    {
        QReadLocker locker(&m_lock);
        m_ogreEngine->activateOgreContext();
        m_pointCloud->updateVertexPositions(m_pDepthData, m_numPoints);
        m_pointCloud->updateVertexColours(m_pColorData, 640*480);
        m_ogreEngine->doneOgreContext();
    }
}

void OgreScene::enableFilter(bool flag)
{
    if (m_chara) {
        m_chara->setVisible(flag);
    }
}
