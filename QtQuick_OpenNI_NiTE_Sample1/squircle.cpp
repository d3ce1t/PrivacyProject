#define GL_WIN_SIZE_X	640
#define GL_WIN_SIZE_Y	480

#include <QElapsedTimer>
#include <QKeyEvent>
#include "squircle.h"
#include <QApplication>
#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>
#include <depthstreamscene.h>

#define MAX_USERS 10

float Squircle::Colors[][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}};
int Squircle::colorCount = 3;
bool g_visibleUsers[MAX_USERS] = {false};
nite::SkeletonState g_skeletonStates[MAX_USERS] = {nite::SKELETON_NONE};
char g_userStatusLabels[MAX_USERS][100] = {{0}};
char g_generalMessage[100] = {0};


Squircle::Squircle()
    : g_poseTimeoutToExit(2000)
{
    g_drawSkeleton = true;
    g_drawCenterOfMass = true;
    g_drawStatusLabel = false;
    g_drawBoundingBox = true;
    g_drawFrameId = false;

    m_t = 0;
    m_thread_t = 0;

    m_program2 = NULL;
    m_pUserTracker = new nite::UserTracker;
    m_depthScene = new DepthStreamScene();

    initOpenNI();
}

Squircle::~Squircle()
{
    if (m_pUserTracker != NULL) {
        delete m_pUserTracker;
        m_pUserTracker = NULL;
    }

    if (m_depthScene != NULL) {
        delete m_depthScene;
        m_depthScene = NULL;
    }

    if (m_program2 != NULL) {
        delete m_program2;
        m_program2 = NULL;
    }
}

void Squircle::setT(qreal t)
{
    if (t == m_t)
        return;
    m_t = t;
    emit tChanged();
    if (window())
        window()->update();
}

void Squircle::itemChange(ItemChange change, const ItemChangeData &)
{
    // The ItemSceneChange event is sent when we are first attached to a window.
    if (change == ItemSceneChange) {
        win = window();
        if (!win)
            return;

        // Connect the beforeRendering signal to our paint function.
        // Since this call is executed on the rendering thread it must be
        // a Qt::DirectConnection
        connect(win, SIGNAL(beforeRendering()), this, SLOT(paint()), Qt::DirectConnection);
        connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(sync()), Qt::DirectConnection);
        connect(win, SIGNAL(widthChanged(int)), this, SLOT(resizeWidth(int)), Qt::DirectConnection);
        connect(win, SIGNAL(heightChanged(int)), this, SLOT(resizeHeight(int)), Qt::DirectConnection);

        // If we allow QML to do the clearing, they would clear what we paint
        // and nothing would show.
        win->setClearBeforeRendering(false);
    }
}

void Squircle::resizeHeight(int height) {
    this->setHeight(height);
}

void Squircle::resizeWidth(int width) {
    this->setWidth(width);
}

// This method must be called from Renderer Thread that own OpenGL context
void Squircle::initialize()
{
    matrix.ortho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);
    m_program2 = new SimpleShaderProgram();
    m_program2->setMatrix(matrix);
    m_depthScene->setNativeResolution(videoMode.getResolutionX(), videoMode.getResolutionY());
    m_depthScene->initialise();

    connect(window()->openglContext(), SIGNAL(aboutToBeDestroyed()),
            this, SLOT(cleanup()), Qt::DirectConnection);

    // Set the clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void Squircle::initOpenNI()
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

void Squircle::paint()
{
    // Read Depth Frame
    nite::UserTrackerFrameRef userTrackerFrame;

    if (m_pUserTracker->readFrame(&userTrackerFrame) != nite::STATUS_OK) {
        printf("GetNextData failed\n");
        return;
    }

    videoMode = userTrackerFrame.getDepthFrame().getVideoMode();

    // Init
    static bool firstTime = true;

    if (firstTime) {
        initialize();
        firstTime = false;
    }

    // Prepare Scene
    m_depthScene->computeVideoTexture(userTrackerFrame);

    //
    // Rendering
    //

    // Configure ViewPort and Clear Screen
    glViewport(0, 0, window()->width(), window()->height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Save State
    glPushMatrix();

    // Draw
    m_depthScene->render();

    const nite::Array<nite::UserData>& users = userTrackerFrame.getUsers();

    for (int i = 0; i < users.getSize(); ++i)
    {
        const nite::UserData& user = users[i];

        updateUserState(user, userTrackerFrame.getTimestamp());

        if (user.isNew()) {
            m_pUserTracker->startSkeletonTracking(user.getId());
            m_pUserTracker->startPoseDetection(user.getId(), nite::POSE_CROSSED_HANDS);
        }
        else if (!user.isLost())
        {
           /* if (g_drawStatusLabel)
            {
                //DrawStatusLabel(m_pUserTracker, user);
            }*/
            if (g_drawCenterOfMass)
            {
                DrawCenterOfMass(m_pUserTracker, user, videoMode);
            }
            if (g_drawBoundingBox)
            {
                DrawBoundingBox(user, videoMode);
            }

            if (users[i].getSkeleton().getState() == nite::SKELETON_TRACKED && g_drawSkeleton)
            {
                DrawSkeleton(m_pUserTracker, user, videoMode);
            }
        }

        if (m_poseUser == 0 || m_poseUser == user.getId())
        {
            const nite::PoseData& pose = user.getPose(nite::POSE_CROSSED_HANDS);

            if (pose.isEntered())
            {
                // Start timer
                sprintf(g_generalMessage, "In exit pose. Keep it for %d second%s to exit\n", g_poseTimeoutToExit/1000, g_poseTimeoutToExit/1000 == 1 ? "" : "s");
                printf("Counting down %d second to exit\n", g_poseTimeoutToExit/1000);
                m_poseUser = user.getId();
                m_poseTime = userTrackerFrame.getTimestamp();
            }
            else if (pose.isExited())
            {
                memset(g_generalMessage, 0, sizeof(g_generalMessage));
                printf("Count-down interrupted\n");
                m_poseTime = 0;
                m_poseUser = 0;
            }
            else if (pose.isHeld())
            {
                // tick
                if (userTrackerFrame.getTimestamp() - m_poseTime > g_poseTimeoutToExit * 1000)
                {
                    printf("Count down complete. Exit...\n");
                    QApplication::exit(2);
                }
            }
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

void Squircle::DrawCenterOfMass(nite::UserTracker* pUserTracker, const nite::UserData& user, const openni::VideoMode& videoMode)
{
    float coordinates[2] = {0};
    const GLfloat centerColour[] = {1.0f, 1.0f, 1.0f};

    pUserTracker->convertJointCoordinatesToDepth(user.getCenterOfMass().x, user.getCenterOfMass().y, user.getCenterOfMass().z, &coordinates[0], &coordinates[1]);

    coordinates[0] *= GL_WIN_SIZE_X/videoMode.getResolutionX();
    coordinates[1] *= GL_WIN_SIZE_Y/videoMode.getResolutionY();

    // Draw
    glPushMatrix();
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    m_program2->bind();
    m_program2->setPosAttribute(coordinates, 2);
    m_program2->setColorAttribute(centerColour, 3);
    m_program2->setPointSize(8);
    m_program2->enableAttributes();

    glDrawArrays(GL_POINTS, m_program2->getPosAttribute(), 1);

    m_program2->disableAttributes();
    m_program2->release();

    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void Squircle::DrawBoundingBox(const nite::UserData& user, const openni::VideoMode& videoMode)
{
    float coordinates[] =
    {
        user.getBoundingBox().max.x, user.getBoundingBox().max.y,
        user.getBoundingBox().max.x, user.getBoundingBox().min.y,
        user.getBoundingBox().min.x, user.getBoundingBox().min.y,
        user.getBoundingBox().min.x, user.getBoundingBox().max.y
    };

    const GLfloat coorColours[] = {
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f
    };

    coordinates[0]  *= GL_WIN_SIZE_X/videoMode.getResolutionX();
    coordinates[1]  *= GL_WIN_SIZE_Y/videoMode.getResolutionY();
    coordinates[2]  *= GL_WIN_SIZE_X/videoMode.getResolutionX();
    coordinates[3]  *= GL_WIN_SIZE_Y/videoMode.getResolutionY();
    coordinates[4]  *= GL_WIN_SIZE_X/videoMode.getResolutionX();
    coordinates[5]  *= GL_WIN_SIZE_Y/videoMode.getResolutionY();
    coordinates[6]  *= GL_WIN_SIZE_X/videoMode.getResolutionX();
    coordinates[7]  *= GL_WIN_SIZE_Y/videoMode.getResolutionY();

    glPushMatrix();
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    //m_program2->setPointSize(2);
    m_program2->bind();
    m_program2->setPosAttribute(coordinates, 2);
    m_program2->setColorAttribute(coorColours, 3);
    m_program2->setPointSize(2);
    m_program2->enableAttributes();

    glDrawArrays(GL_LINE_LOOP, m_program2->getPosAttribute(), 4);

    m_program2->disableAttributes();
    m_program2->release();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glPopMatrix();
}

void Squircle::DrawLimb(nite::UserTracker* pUserTracker, const nite::SkeletonJoint& joint1, const nite::SkeletonJoint& joint2, int color, const openni::VideoMode& videoMode)
{
    if (joint1.getPositionConfidence() < 0.5f || joint2.getPositionConfidence() < 0.5f)
        return;

    float coordinates[4] = {0};
    pUserTracker->convertJointCoordinatesToDepth(joint1.getPosition().x, joint1.getPosition().y, joint1.getPosition().z, &coordinates[0], &coordinates[1]);
    pUserTracker->convertJointCoordinatesToDepth(joint2.getPosition().x, joint2.getPosition().y, joint2.getPosition().z, &coordinates[2], &coordinates[3]);

    coordinates[0] *= GL_WIN_SIZE_X/videoMode.getResolutionX();
    coordinates[1] *= GL_WIN_SIZE_Y/videoMode.getResolutionY();
    coordinates[2] *= GL_WIN_SIZE_X/videoMode.getResolutionX();
    coordinates[3] *= GL_WIN_SIZE_Y/videoMode.getResolutionY();

    float factor = (joint1.getPositionConfidence() + joint2.getPositionConfidence()) / 2.0f;

    float coorColours[3] = {Colors[color][0] * factor,
                            Colors[color][1] * factor,
                            Colors[color][2] * factor};

    glPushMatrix();
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    // Bind shader
    m_program2->bind();

    // Draw Line from joint1 to joint2
    m_program2->setPointSize(2);
    m_program2->setPosAttribute(coordinates, 2);
    m_program2->setColorAttribute(coorColours, 3);
    m_program2->enableAttributes();
    glDrawArrays(GL_LINES, m_program2->getPosAttribute(), 2);

    // Draw point for joint1
    coorColours[0] = Colors[color][0] * joint1.getPositionConfidence();
    coorColours[1] = Colors[color][0] * joint1.getPositionConfidence();
    coorColours[2] = Colors[color][0] * joint1.getPositionConfidence();
    m_program2->setPointSize(10);
    m_program2->setColorAttribute(coorColours, 3);
    glDrawArrays(GL_POINTS, m_program2->getPosAttribute(), 1);

    // Draw point for joint2
    coorColours[0] = Colors[color][0] * joint2.getPositionConfidence();
    coorColours[1] = Colors[color][0] * joint2.getPositionConfidence();
    coorColours[2] = Colors[color][0] * joint2.getPositionConfidence();
    m_program2->setColorAttribute(coorColours, 3);
    m_program2->setPosAttribute(coordinates+2, 2);
    glDrawArrays(GL_POINTS, m_program2->getPosAttribute(), 1);

    // Release shader
    m_program2->disableAttributes();
    m_program2->release();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glPopMatrix();
}

void Squircle::cleanup()
{
    nite::NiTE::shutdown();
    m_device.close();
    openni::OpenNI::shutdown();
}

void Squircle::sync()
{
    m_thread_t = m_t;
}

void Squircle::printMessage(const nite::UserData &user, uint64_t ts, const char *msg)
{
    sprintf(g_userStatusLabels[user.getId()],"%s", msg);
    printf("[%08llu] User #%d:\t%s\n",ts, user.getId(),msg);
}

void Squircle::updateUserState(const nite::UserData& user, uint64_t ts)
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

/*void Squircle::DrawStatusLabel(nite::UserTracker* pUserTracker, const nite::UserData& user)
{
    int color = user.getId() % colorCount;
    glColor3f(1.0f - Colors[color][0], 1.0f - Colors[color][1], 1.0f - Colors[color][2]);

    float x,y;
    pUserTracker->convertJointCoordinatesToDepth(user.getCenterOfMass().x, user.getCenterOfMass().y, user.getCenterOfMass().z, &x, &y);
    x *= GL_WIN_SIZE_X/g_nXRes;
    y *= GL_WIN_SIZE_Y/g_nYRes;
    char *msg = g_userStatusLabels[user.getId()];
    glRasterPos2i(x-((strlen(msg)/2)*8),y);
    //glPrintString(GLUT_BITMAP_HELVETICA_18, msg);
}*/

/*void Squircle::DrawFrameId(int frameId)
{
    char buffer[80] = "";
    sprintf(buffer, "%d", frameId);
    glColor3f(1.0f, 0.0f, 0.0f);
    glRasterPos2i(20, 20);
    //glPrintString(GLUT_BITMAP_HELVETICA_18, buffer);
}*/

void Squircle::DrawSkeleton(nite::UserTracker* pUserTracker, const nite::UserData& userData, const openni::VideoMode& videoMode)
{
    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_HEAD), userData.getSkeleton().getJoint(nite::JOINT_NECK), userData.getId() % colorCount, videoMode);

    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_SHOULDER), userData.getSkeleton().getJoint(nite::JOINT_LEFT_ELBOW), userData.getId() % colorCount, videoMode);
    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_ELBOW), userData.getSkeleton().getJoint(nite::JOINT_LEFT_HAND), userData.getId() % colorCount, videoMode);

    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_SHOULDER), userData.getSkeleton().getJoint(nite::JOINT_RIGHT_ELBOW), userData.getId() % colorCount, videoMode);
    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_ELBOW), userData.getSkeleton().getJoint(nite::JOINT_RIGHT_HAND), userData.getId() % colorCount, videoMode);

    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_SHOULDER), userData.getSkeleton().getJoint(nite::JOINT_RIGHT_SHOULDER), userData.getId() % colorCount, videoMode);

    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_SHOULDER), userData.getSkeleton().getJoint(nite::JOINT_TORSO), userData.getId() % colorCount, videoMode);
    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_SHOULDER), userData.getSkeleton().getJoint(nite::JOINT_TORSO), userData.getId() % colorCount, videoMode);

    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_TORSO), userData.getSkeleton().getJoint(nite::JOINT_LEFT_HIP), userData.getId() % colorCount, videoMode);
    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_TORSO), userData.getSkeleton().getJoint(nite::JOINT_RIGHT_HIP), userData.getId() % colorCount, videoMode);

    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_HIP), userData.getSkeleton().getJoint(nite::JOINT_RIGHT_HIP), userData.getId() % colorCount, videoMode);


    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_HIP), userData.getSkeleton().getJoint(nite::JOINT_LEFT_KNEE), userData.getId() % colorCount, videoMode);
    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_KNEE), userData.getSkeleton().getJoint(nite::JOINT_LEFT_FOOT), userData.getId() % colorCount, videoMode);

    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_HIP), userData.getSkeleton().getJoint(nite::JOINT_RIGHT_KNEE), userData.getId() % colorCount, videoMode);
    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_KNEE), userData.getSkeleton().getJoint(nite::JOINT_RIGHT_FOOT), userData.getId() % colorCount, videoMode);
}
