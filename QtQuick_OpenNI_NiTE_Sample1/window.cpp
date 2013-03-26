#include "window.h"
#include <basicusagescene.h>
#include <depthstreamscene.h>
#include <QTimer>
#include <QDebug>
#include <QQmlContext>
#include <QtQml>
#include <iostream>
#include <grill.h>
#include <histogramscene.h>

Window::Window( QWindow *parent )
    : QQuickView( parent ), g_poseTimeoutToExit(2000)
{
    m_statusLabel = new QString("Not tracking");
    g_drawStatusLabel = true;
    g_drawFrameId = true;
    g_drawHistogram = false;
    m_frameId = 0;
    m_renderedFrames = 0;

    memset(g_userStatusLabels, 0, MAX_USERS*100*sizeof(char));

    for (int i=0; i<MAX_USERS; ++i) {
        g_visibleUsers[i] = false;
        g_skeletonStates[i] =  nite::SKELETON_NONE;
    }

    setClearBeforeRendering( false );

    QObject::connect(this, SIGNAL(beforeRendering()), SLOT(renderOpenGLScene()), Qt::DirectConnection);

    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(update()));
    timer->start( 33 );

    m_depthScene = new DepthStreamScene();
    m_scene = new BasicUsageScene();
    m_grill = new Grill();
    m_histogram = new HistogramScene;

    //m_pUserTracker = new nite::UserTracker();
    initOpenNI();

    // QML properties
    rootContext()->setContextProperty("appSettings1", (QObject *) m_scene);
    rootContext()->setContextProperty("winObject", (QObject *) this);
    rootContext()->setContextProperty("appSettings2", (QObject *) m_depthScene);
    rootContext()->setContextProperty("skeleton", (QObject *) &m_scene->getSkeleton());
}

Window::~Window()
{
    nite::NiTE::shutdown();
    m_device.close();
    openni::OpenNI::shutdown();

    /*if (m_pUserTracker != NULL) {
        delete m_pUserTracker;
        m_pUserTracker = NULL;
    }*/

    if (m_depthScene != NULL) {
        delete m_depthScene;
        m_depthScene = NULL;
    }

    if (m_scene != NULL) {
        delete m_scene;
        m_scene = NULL;
    }

    if (m_grill != NULL) {
        delete m_grill;
        m_grill = NULL;
    }

    if (m_histogram != NULL) {
        delete m_histogram;
        m_histogram = NULL;
    }
}

void Window::initialise()
{
    //QSurfaceFormat format;
    //format.setMajorVersion(2);
    //format.setMinorVersion(0);
    //format.setSamples(4);
    //format.setDepthBufferSize(32);

    //format.setRenderableType(QSurfaceFormat::OpenGLES);
    //format.setProfile( QSurfaceFormat::CoreProfile);
    //setFormat( format );

    int depth;
    glGetIntegerv(GL_DEPTH_BITS, &depth);
    qDebug() << "Width: " << width() << " Height: " << height() << " Depth: " << depth;

    // Scenes
    m_depthScene->setNativeResolution(videoMode.getResolutionX(), videoMode.getResolutionY());
    m_depthScene->resize(width(), height());
    m_depthScene->initialise();

    m_scene->setNativeResolution(videoMode.getResolutionX(), videoMode.getResolutionY());
    m_scene->resize(width(), height());
    m_scene->initialise();

    m_grill->initialise();

    m_histogram->initialise();

    resetPerspective();
    m_time.start();
}

void Window::renderOpenGLScene()
{
    // Read Depth Frame
    nite::UserTrackerFrameRef userTrackerFrame;

    m_pUserTracker.setSkeletonSmoothingFactor(0.7);

    if (m_pUserTracker.readFrame(&userTrackerFrame) != nite::STATUS_OK) {
        printf("GetNextData failed\n");
        return;
    }

    m_colorStream.readFrame(&m_colorFrame);

    videoMode = userTrackerFrame.getDepthFrame().getVideoMode();

    static bool firstTime = true;

    // First init (depends of video mode)
    // Make here in order to use the OpenGLContext initalised by QtQuick
    if ( firstTime )
    {
        initialise();
        firstTime = false;
    }

    // Prepare Scene
    m_depthScene->computeVideoTexture(userTrackerFrame, m_colorFrame);
    m_histogram->setHistogram(m_depthScene->getDepthHistogram());
    const nite::Array<nite::UserData>& users = userTrackerFrame.getUsers();

    //
    // Rendering
    //

    // Init Each Frame (because QtQuirck could change it)
    glDepthRange(0.0f, 1.0f);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    // Configure ViewPort and Clear Screen
    glViewport(0, 0, width(), height());
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Render
    m_depthScene->render();

    m_grill->render();

    if (g_drawHistogram)
        m_histogram->render();

    int numberOfUsers = MAX_USERS <= users.getSize()? MAX_USERS : users.getSize();

    for (int i = 0; i < numberOfUsers; ++i)
    {
        const nite::UserData& user = users[i];
        updateUserState(user, userTrackerFrame.getTimestamp());

        if (user.isNew()) {
            m_pUserTracker.startSkeletonTracking(user.getId());
            m_pUserTracker.startPoseDetection(user.getId(), nite::POSE_CROSSED_HANDS);
        }
        else if (!user.isLost()) {
            m_scene->setUser(user);
            m_scene->setUserTrackerPointer(&m_pUserTracker);
            m_scene->render();
        }
    }

    // Compute Frame Per Seconds
    m_frameId = userTrackerFrame.getFrameIndex();
    m_fps = ++m_renderedFrames / (m_time.elapsed() / 1000.0f);
    emit changeOfStatus();

    // Restore
    glDisable(GL_DEPTH_TEST);
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

void Window::resetPerspective()
{
    matrix.setToIdentity();
    matrix.perspective(70, 1.0, 0.01, 10.0);
    matrix.translate(0, 0, -1.1);
    m_grill->setMatrix(matrix);
    m_depthScene->setMatrix(matrix);
    m_histogram->setMatrix(matrix);
    m_scene->setMatrix(matrix);
}

void Window::rotateAxisX(float angle)
{
    matrix.rotate(angle, QVector3D(1, 0, 0));
    m_grill->setMatrix(matrix);
    m_depthScene->setMatrix(matrix);
    m_histogram->setMatrix(matrix);
    m_scene->setMatrix(matrix);
}

void Window::rotateAxisY(float angle)
{
    matrix.rotate(angle, QVector3D(0, 1, 0));
    m_grill->setMatrix(matrix);
    m_depthScene->setMatrix(matrix);
    m_histogram->setMatrix(matrix);
    m_scene->setMatrix(matrix);
}

void Window::rotateAxisZ(float angle)
{
    matrix.rotate(angle, QVector3D(0, 0, 1));
    m_grill->setMatrix(matrix);
    m_depthScene->setMatrix(matrix);
    m_histogram->setMatrix(matrix);
    m_scene->setMatrix(matrix);
}

void Window::translateAxisX(float value)
{
    matrix.translate(value, 0, 0);
    m_grill->setMatrix(matrix);
    m_depthScene->setMatrix(matrix);
    m_histogram->setMatrix(matrix);
    m_scene->setMatrix(matrix);
}

void Window::translateAxisY(float value)
{
    matrix.translate(0, value, 0);
    m_grill->setMatrix(matrix);
    m_depthScene->setMatrix(matrix);
    m_histogram->setMatrix(matrix);
    m_scene->setMatrix(matrix);
}

void Window::translateAxisZ(float value)
{
    matrix.translate(0, 0, value);
    m_grill->setMatrix(matrix);
    m_depthScene->setMatrix(matrix);
    m_histogram->setMatrix(matrix);
    m_scene->setMatrix(matrix);
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

        if (m_colorStream.create(m_device, openni::SENSOR_COLOR) != openni::STATUS_OK)
            throw 4;

        if (m_colorStream.start() != openni::STATUS_OK)
            throw 5;

        if (m_pUserTracker.create(&m_device) != nite::STATUS_OK)
            throw 6;

        if (!m_pUserTracker.isValid() || !m_colorStream.isValid())
            throw 7;
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
    *m_statusLabel = msg;
    emit changeOfStatus();
    printf("[%08lu] User #%d:\t%s\n", ts, user.getId(),msg);
}

void Window::updateUserState(const nite::UserData& user, uint64_t ts)
{
    if (user.isNew())
        printMessage(user, ts, "New");
    else if (user.isVisible() && !g_visibleUsers[user.getId()])
        printf("[%08lu] User #%d:\tVisible\n", ts, user.getId());
    else if (!user.isVisible() && g_visibleUsers[user.getId()])
        printf("[%08lu] User #%d:\tOut of Scene\n", ts, user.getId());
    else if (user.isLost())
        printMessage(user, ts, "Lost");

    g_visibleUsers[user.getId()] = user.isVisible();


    if (g_skeletonStates[user.getId()] != user.getSkeleton().getState())
    {
        switch(g_skeletonStates[user.getId()] = user.getSkeleton().getState())
        {
        case nite::SKELETON_NONE:
            printMessage(user, ts, "Stopped tracking.");
            m_scene->getSkeleton().setState(0);
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

void Window::setDrawStatusLabelFlag(bool value) {
    g_drawStatusLabel = value;
    emit changeOfStatus();
}

void Window::setDrawHistogramFlag (bool value) {
    g_drawHistogram = value;
    emit changeOfStatus();
}

void Window::setDrawFrameIdFlag(bool value) {
    g_drawFrameId = value;
    emit changeOfStatus();
}
