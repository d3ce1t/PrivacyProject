#include "window.h"
#include <basicusagescene.h>
#include <depthstreamscene.h>
#include <QTimer>
#include <QDebug>
#include <QQmlContext>
#include <QtQml>
#include <iostream>
#include <grill.h>

Window::Window( QWindow *parent )
    : QQuickView( parent ), g_poseTimeoutToExit(2000)
{
    m_statusLabel = new QString("Not tracking");
    g_drawStatusLabel = true;
    g_drawFrameId = true;
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

    QSurfaceFormat format;
    format.setMajorVersion(2);
    format.setMinorVersion(0);
    format.setSamples(4);
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setProfile( QSurfaceFormat::CoreProfile);
    setFormat( format );

    m_scene = new BasicUsageScene();
    m_depthScene = new DepthStreamScene();
    m_pUserTracker = new nite::UserTracker();
    m_grill = new Grill();
    //initOpenNI();

    // QML properties
    rootContext()->setContextProperty("appSettings1", (QObject *) m_scene);
    rootContext()->setContextProperty("appSettings2", (QObject *) this);
    rootContext()->setContextProperty("skeleton", (QObject *) &m_scene->getSkeleton());
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

    if (m_grill != NULL) {
        delete m_grill;
        m_grill = NULL;
    }
}

void Window::renderOpenGLScene()
{
    // Read Depth Frame
    /*nite::UserTrackerFrameRef userTrackerFrame;

    m_pUserTracker->setSkeletonSmoothingFactor(0.7);

    if (m_pUserTracker->readFrame(&userTrackerFrame) != nite::STATUS_OK) {
        printf("GetNextData failed\n");
        return;
    }

    videoMode = userTrackerFrame.getDepthFrame().getVideoMode();*/

    static bool firstTime = true;

    // First init (depends of video mode)
    // Make here in order to use the OpenGLContext initalised by QtQuick
    if ( firstTime )
    {
        matrix.perspective(70, 1.0, 0.0, 100.0);
        matrix.translate(0, -0.5, -2);
        //matrix.rotate(45, QVector3D(0, 1, 0));
        //matrix.ortho(0, width(), height(), 0, -1.0, 1.0);
        /*m_depthScene->setNativeResolution(videoMode.getResolutionX(), videoMode.getResolutionY());
        m_depthScene->setMatrix(matrix);
        m_depthScene->resize(width(), height());
        m_depthScene->initialise();
        m_scene->setNativeResolution(videoMode.getResolutionX(), videoMode.getResolutionY());
        m_scene->setMatrix(matrix);
        m_scene->resize(width(), height());
        m_scene->initialise();*/
        m_grill->setMatrix(matrix);
        m_grill->initialise();
        m_time.start();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        qDebug() << "Width: " << width() << " Height: " << height();
        firstTime = false;
    }

    // Prepare Scene
    //m_depthScene->computeVideoTexture(userTrackerFrame);
    //const nite::Array<nite::UserData>& users = userTrackerFrame.getUsers();

    //
    // Rendering
    //

    // Configure ViewPort and Clear Screen
    glViewport(0, 0, width(), height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Save State
    glPushMatrix();

    // Render
    //m_depthScene->render();

    m_grill->render();

   /* int numberOfUsers = MAX_USERS <= users.getSize()? MAX_USERS : users.getSize();

    for (int i = 0; i < numberOfUsers; ++i)
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
    }*/

    // Compute Frame Per Seconds
    //m_frameId = userTrackerFrame.getFrameIndex();
    m_fps = ++m_renderedFrames / (m_time.elapsed() / 1000.0f);
    emit changeOfStatus();

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

/*void Window::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Left:
        matrix.rotate(2, QVector3D(0, 1, 0));
        qDebug() << "Left";
        break;

    case Qt::Key_Right:
        matrix.rotate(-2, QVector3D(0, 1, 0));
        qDebug() << "Right";
        break;

    case Qt::Key_Up:
        matrix.rotate(2, QVector3D(1, 0, 0));
        qDebug() << "Up";
        break;

    case Qt::Key_Down:
        matrix.rotate(-2, QVector3D(1, 0, 0));
        qDebug() << "Down";
        break;

    case Qt::Key_A:
        matrix.translate(-0.1, 0, 0);
        qDebug() << "Left";
        break;

    case Qt::Key_D:
        matrix.translate(+0.1, 0, 0);
        qDebug() << "Right";
        break;

    case Qt::Key_W:
        matrix.translate(0, 0, 0.1);
        qDebug() << "Up";
        break;

    case Qt::Key_S:
        matrix.translate(0, 0, -0.1);
        qDebug() << "Down";
        break;
    }

    m_grill->setMatrix(matrix);

}*/

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

void Window::setDrawFrameIdFlag(bool value) {
    g_drawFrameId = value;
    emit changeOfStatus();
}
