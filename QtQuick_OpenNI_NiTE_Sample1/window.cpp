#include "window.h"
#include <basicusagescene.h>
#include <depthstreamscene.h>
#include <QTimer>
#include <QDebug>
#include <QQmlContext>


Window::Window( QWindow *parent )
    : QQuickView( parent ), g_poseTimeoutToExit(2000)
{
    memset(g_userStatusLabels, 0, MAX_USERS*100*sizeof(char));

    for (int i=0; i<MAX_USERS; ++i) {
        g_visibleUsers[i] = false;
        g_skeletonStates[i] =  nite::SKELETON_NONE;
    }

    setClearBeforeRendering( false );

    QObject::connect(this, SIGNAL(beforeRendering()), SLOT(renderOpenGLScene()), Qt::DirectConnection);

    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(update()));
    timer->start( 16 );

    QSurfaceFormat format;
    format.setMajorVersion(2);
    format.setMinorVersion(0);
    format.setSamples(4);
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setProfile( QSurfaceFormat::CoreProfile);
    setFormat( format );

    m_scene = new BasicUsageScene();
    m_depthScene = new DepthStreamScene();
    m_pUserTracker = new nite::UserTracker;
    initOpenNI();

    // QML properties
    rootContext()->setContextProperty("rootItem", (QObject *) m_scene);
}

Window::~Window()
{
    nite::NiTE::shutdown();
    m_device.close();
    openni::OpenNI::shutdown();

    if (m_pUserTracker != NULL) {
        delete m_pUserTracker;
        m_pUserTracker = NULL;
    }

    if (m_depthScene != NULL) {
        delete m_depthScene;
        m_depthScene = NULL;
    }

    if (m_scene != NULL) {
        delete m_scene;
        m_scene = NULL;
    }
}

void Window::renderOpenGLScene()
{
    // Read Depth Frame
    nite::UserTrackerFrameRef userTrackerFrame;

    if (m_pUserTracker->readFrame(&userTrackerFrame) != nite::STATUS_OK) {
        printf("GetNextData failed\n");
        return;
    }

    videoMode = userTrackerFrame.getDepthFrame().getVideoMode();

    static bool firstTime = true;

    // First init (depends of video mode)
    // Make here in order to use the OpenGLContext initalised by QtQuick
    if ( firstTime )
    {
        matrix.ortho(0, videoMode.getResolutionX(), videoMode.getResolutionY(), 0, -1.0, 1.0);
        m_depthScene->setNativeResolution(videoMode.getResolutionX(), videoMode.getResolutionY());
        m_depthScene->setMatrix(matrix);
        m_depthScene->resize(width(), height());
        m_depthScene->initialise();
        m_scene->setNativeResolution(videoMode.getResolutionX(), videoMode.getResolutionY());
        m_scene->setMatrix(matrix);
        m_scene->resize(width(), height());
        m_scene->initialise();
        m_time.start();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        qDebug() << "Width: " << width() << " Height: " << height();
        firstTime = false;
    }

    // Prepare Scene
    m_depthScene->computeVideoTexture(userTrackerFrame);
    const nite::Array<nite::UserData>& users = userTrackerFrame.getUsers();

    //
    // Rendering
    //

    // Configure ViewPort and Clear Screen

    glViewport(0, 0, width(), height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Save State
    glPushMatrix();

    // Render
    m_depthScene->render();

    for (int i = 0; i < users.getSize(); ++i)
    {
        const nite::UserData& user = users[i];
        updateUserState(user, userTrackerFrame.getTimestamp());
        if (user.isNew()) {
            m_pUserTracker->startSkeletonTracking(user.getId());
            m_pUserTracker->startPoseDetection(user.getId(), nite::POSE_CROSSED_HANDS);
        }
        else if (!user.isLost()) {
            m_scene->setUser(user);
            m_scene->setUserTrackerPointer(m_pUserTracker);
            m_scene->render();
        }
    }

    /*if (g_drawFrameId)
    {
        DrawFrameId(userTrackerFrame.getFrameIndex());
    }*/

    /*if (g_generalMessage[0] != '\0')
    {
        char *msg = g_generalMessage;
        glColor3f(1.0f, 0.0f, 0.0f);
        glRasterPos2i(100, 20);

        glPrintString(GLUT_BITMAP_HELVETICA_18, msg);
    }*/

    // Restore State
    glPopMatrix();
}

void Window::update()
{
    float time = m_time.elapsed() / 1000.0f;
    m_scene->update( time );
    QQuickView::update();
}

void Window::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed()) {
        /*matrix.ortho(0, width(), height(), 0, -1.0, 1.0);
        m_depthScene->setMatrix(matrix);
        m_scene->setMatrix(matrix);*/
        qDebug() << "New size: " << width() << " Height:" << height();
    }
}

void Window::initOpenNI()
{
    const char* deviceURI = openni::ANY_DEVICE;

    try {
        if (openni::OpenNI::initialize() != openni::STATUS_OK)
            throw 1;

        if (m_device.open(deviceURI) != openni::STATUS_OK)
            throw 2;

        if (nite::NiTE::initialize() != nite::STATUS_OK)
            throw 3;

        if (m_pUserTracker->create(&m_device) != nite::STATUS_OK)
            throw 4;
    }
    catch (int ex)
    {
        printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
        nite::NiTE::shutdown();
        openni::OpenNI::shutdown();
        throw ex;
    }
}

void Window::printMessage(const nite::UserData &user, uint64_t ts, const char *msg)
{
    sprintf(g_userStatusLabels[user.getId()],"%s", msg);
    printf("[%08llu] User #%d:\t%s\n",ts, user.getId(),msg);
}

void Window::updateUserState(const nite::UserData& user, uint64_t ts)
{
    if (user.isNew())
        printMessage(user, ts, "New");
    else if (user.isVisible() && !g_visibleUsers[user.getId()])
        printf("[%08llu] User #%d:\tVisible\n", ts, user.getId());
    else if (!user.isVisible() && g_visibleUsers[user.getId()])
        printf("[%08llu] User #%d:\tOut of Scene\n", ts, user.getId());
    else if (user.isLost())
        printMessage(user, ts, "Lost");

    g_visibleUsers[user.getId()] = user.isVisible();


    if(g_skeletonStates[user.getId()] != user.getSkeleton().getState())
    {
        switch(g_skeletonStates[user.getId()] = user.getSkeleton().getState())
        {
        case nite::SKELETON_NONE:
            printMessage(user, ts, "Stopped tracking.");
            break;
        case nite::SKELETON_CALIBRATING:
            printMessage(user, ts, "Calibrating...");
            break;
        case nite::SKELETON_TRACKED:
            printMessage(user, ts, "Tracking!");
            break;
        case nite::SKELETON_CALIBRATION_ERROR_NOT_IN_POSE:
        case nite::SKELETON_CALIBRATION_ERROR_HANDS:
        case nite::SKELETON_CALIBRATION_ERROR_LEGS:
        case nite::SKELETON_CALIBRATION_ERROR_HEAD:
        case nite::SKELETON_CALIBRATION_ERROR_TORSO:
            printMessage(user,ts, "Calibration Failed... :-|");
            break;
        }
    }
}
