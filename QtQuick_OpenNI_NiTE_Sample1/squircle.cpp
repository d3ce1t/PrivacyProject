#define GL_WIN_SIZE_X	640
#define GL_WIN_SIZE_Y	480
#define TEXTURE_SIZE	512
#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_DEPTH

#define MIN_NUM_CHUNKS(data_size, chunk_size)	((((data_size)-1) / (chunk_size) + 1))
#define MIN_CHUNKS_SIZE(data_size, chunk_size)	(MIN_NUM_CHUNKS(data_size, chunk_size) * (chunk_size))

#include <iostream>
#include <QElapsedTimer>
#include <QKeyEvent>
#include "OniSampleUtilities.h"
#include "squircle.h"
#include <QApplication>

#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>

bool g_drawSkeleton = true;
bool g_drawCenterOfMass = true;
bool g_drawStatusLabel = false;
bool g_drawBoundingBox = true;
bool g_drawBackground = true;
bool g_drawDepth = true;
bool g_drawFrameId = false;

int g_nXRes = 0, g_nYRes = 0;

float Colors[][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}};
int colorCount = 3;
#define MAX_USERS 10
bool g_visibleUsers[MAX_USERS] = {false};
nite::SkeletonState g_skeletonStates[MAX_USERS] = {nite::SKELETON_NONE};
char g_userStatusLabels[MAX_USERS][100] = {{0}};

char g_generalMessage[100] = {0};

// time to hold in pose to exit program. In milliseconds.
const int g_poseTimeoutToExit = 2000;
const int textureUnit = 0;

float vertices[] = {
    0, 0,
    GL_WIN_SIZE_X, 0,
    GL_WIN_SIZE_X, GL_WIN_SIZE_Y,
    0, GL_WIN_SIZE_Y
};


GLfloat testVertices[] = {
    0.0f, 0.707f,
    -0.5f, -0.5f,
    0.5f, -0.5f
};

const GLfloat testColours[] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

using namespace std;


Squircle::Squircle()
{
    m_initialized_flag = false;
    m_t = 0;
    m_thread_t = 0;
    m_pTexMap = NULL;
    m_program1 = NULL;
    m_program2 = NULL;
    m_pUserTracker = new nite::UserTracker;
    initOpenNI();
}

Squircle::~Squircle()
{
    if (m_pUserTracker != NULL) {
        delete m_pUserTracker;
        m_pUserTracker = NULL;
    }

    if (m_pTexMap != NULL) {
        delete[] m_pTexMap;
        m_pTexMap = NULL;
    }

    if (m_program1 != NULL) {
        delete m_program1;
    }

    if (m_program2 != NULL) {
        delete m_program2;
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
    m_program1 = new TextureShaderProgram();
    m_program1->setMatrix(matrix);
    m_program2 = new SimpleShaderProgram();
    m_program2->setMatrix(matrix);
    glGenTextures(1, &m_frameTexture);

    connect(window()->openglContext(), SIGNAL(aboutToBeDestroyed()),
            this, SLOT(cleanup()), Qt::DirectConnection);
}

void Squircle::paint()
{
    // Init
    if (!m_initialized_flag) {
        initialize();
        m_initialized_flag = true;
    }

    // Read Depth Frame
    nite::UserTrackerFrameRef userTrackerFrame;

    if (m_pUserTracker->readFrame(&userTrackerFrame) != nite::STATUS_OK) {
        printf("GetNextData failed\n");
        return;
    }

    // Compute Texture Frame (calc hist of depth frame and load it into GPU as a texture)
    computeVideoTexture(userTrackerFrame);

    //
    // Rendering
    //

    // Configure ViewPort and Clear Screen
    glViewport(0, 0, window()->width(), window()->height());
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Save State
    glPushMatrix();

    // Draw
    DrawDepthFrame(userTrackerFrame.getDepthFrame());

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
                DrawCenterOfMass(m_pUserTracker, user);
            }
            if (g_drawBoundingBox)
            {
                DrawBoundingBox(user);
            }

            if (users[i].getSkeleton().getState() == nite::SKELETON_TRACKED && g_drawSkeleton)
            {
                DrawSkeleton(m_pUserTracker, user);
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

void Squircle::DrawDepthFrame(const openni::VideoFrameRef& depthFrame)
{
    float textCoords[] = {
        0, 0,
        (float)g_nXRes/(float)m_nTexMapX, 0,
        (float)g_nXRes/(float)m_nTexMapX, (float)g_nYRes/(float)m_nTexMapY,
        0, (float)g_nYRes/(float)m_nTexMapY
    };

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glPushMatrix();

    m_program1->bind();

    m_program1->setPosAttribute(vertices, 2);
    m_program1->setTexCoordAttribute(textCoords, 2);
    m_program1->enableAttributes();

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_frameTexture);
    glDrawArrays(GL_TRIANGLE_FAN, m_program1->getPosAttribute(), 4);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_program1->disableAttributes();
    m_program1->release();

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);
}

void Squircle::DrawCenterOfMass(nite::UserTracker* pUserTracker, const nite::UserData& user)
{
    float coordinates[2] = {0};
    const GLfloat centerColour[] = {1.0f, 1.0f, 1.0f};

    pUserTracker->convertJointCoordinatesToDepth(user.getCenterOfMass().x, user.getCenterOfMass().y, user.getCenterOfMass().z, &coordinates[0], &coordinates[1]);

    coordinates[0] *= GL_WIN_SIZE_X/g_nXRes;
    coordinates[1] *= GL_WIN_SIZE_Y/g_nYRes;

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

void Squircle::DrawBoundingBox(const nite::UserData& user)
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

    coordinates[0]  *= GL_WIN_SIZE_X/g_nXRes;
    coordinates[1]  *= GL_WIN_SIZE_Y/g_nYRes;
    coordinates[2]  *= GL_WIN_SIZE_X/g_nXRes;
    coordinates[3]  *= GL_WIN_SIZE_Y/g_nYRes;
    coordinates[4]  *= GL_WIN_SIZE_X/g_nXRes;
    coordinates[5]  *= GL_WIN_SIZE_Y/g_nYRes;
    coordinates[6]  *= GL_WIN_SIZE_X/g_nXRes;
    coordinates[7]  *= GL_WIN_SIZE_Y/g_nYRes;

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

void Squircle::DrawLimb(nite::UserTracker* pUserTracker, const nite::SkeletonJoint& joint1, const nite::SkeletonJoint& joint2, int color)
{
    if (joint1.getPositionConfidence() < 0.5f || joint2.getPositionConfidence() < 0.5f)
        return;

    float coordinates[4] = {0};
    pUserTracker->convertJointCoordinatesToDepth(joint1.getPosition().x, joint1.getPosition().y, joint1.getPosition().z, &coordinates[0], &coordinates[1]);
    pUserTracker->convertJointCoordinatesToDepth(joint2.getPosition().x, joint2.getPosition().y, joint2.getPosition().z, &coordinates[2], &coordinates[3]);

    coordinates[0] *= GL_WIN_SIZE_X/g_nXRes;
    coordinates[1] *= GL_WIN_SIZE_Y/g_nYRes;
    coordinates[2] *= GL_WIN_SIZE_X/g_nXRes;
    coordinates[3] *= GL_WIN_SIZE_Y/g_nYRes;

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

void Squircle::initOpenNI()
{
    const char* deviceURI = openni::ANY_DEVICE;

    try {
        if (openni::OpenNI::initialize() != openni::STATUS_OK)
            throw 1;

        if (m_device.open(deviceURI) != openni::STATUS_OK)
            throw 2;

       /* if (m_depthStream.create(m_device, openni::SENSOR_DEPTH) != openni::STATUS_OK)
            throw 3;

        if (m_depthStream.start() != openni::STATUS_OK)
            throw 5;*/

        if (nite::NiTE::initialize() != nite::STATUS_OK)
            throw 4;

        if (m_pUserTracker->create(&m_device) != nite::STATUS_OK)
            throw 5;
    }
    catch (int ex)
    {
        printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
        nite::NiTE::shutdown();
        openni::OpenNI::shutdown();
        throw ex;
    }
}

void Squircle::keyPressEvent(QKeyEvent* ev) {
    switch (ev->key())
    {
    case Qt::Key_Escape:
        QApplication::exit();
        break;
    case Qt::Key_S:
        // Draw skeleton?
        g_drawSkeleton = !g_drawSkeleton;
        break;
    case Qt::Key_L:
        // Draw user status label?
        g_drawStatusLabel = !g_drawStatusLabel;
        break;
    case Qt::Key_C:
        // Draw center of mass?
        g_drawCenterOfMass = !g_drawCenterOfMass;
        break;
    case Qt::Key_X:
        // Draw bounding box?
        g_drawBoundingBox = !g_drawBoundingBox;
        break;
    case Qt::Key_B:
        // Draw background?
        g_drawBackground = !g_drawBackground;
        break;
    case Qt::Key_D:
        // Draw depth?
        g_drawDepth = !g_drawDepth;
        break;
    case Qt::Key_F:
        // Draw frame ID
        g_drawFrameId = !g_drawFrameId;
        break;
    }
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

void Squircle::DrawStatusLabel(nite::UserTracker* pUserTracker, const nite::UserData& user)
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
}

void Squircle::DrawFrameId(int frameId)
{
    char buffer[80] = "";
    sprintf(buffer, "%d", frameId);
    glColor3f(1.0f, 0.0f, 0.0f);
    glRasterPos2i(20, 20);
    //glPrintString(GLUT_BITMAP_HELVETICA_18, buffer);
}

void Squircle::DrawSkeleton(nite::UserTracker* pUserTracker, const nite::UserData& userData)
{
    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_HEAD), userData.getSkeleton().getJoint(nite::JOINT_NECK), userData.getId() % colorCount);

    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_SHOULDER), userData.getSkeleton().getJoint(nite::JOINT_LEFT_ELBOW), userData.getId() % colorCount);
    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_ELBOW), userData.getSkeleton().getJoint(nite::JOINT_LEFT_HAND), userData.getId() % colorCount);

    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_SHOULDER), userData.getSkeleton().getJoint(nite::JOINT_RIGHT_ELBOW), userData.getId() % colorCount);
    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_ELBOW), userData.getSkeleton().getJoint(nite::JOINT_RIGHT_HAND), userData.getId() % colorCount);

    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_SHOULDER), userData.getSkeleton().getJoint(nite::JOINT_RIGHT_SHOULDER), userData.getId() % colorCount);

    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_SHOULDER), userData.getSkeleton().getJoint(nite::JOINT_TORSO), userData.getId() % colorCount);
    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_SHOULDER), userData.getSkeleton().getJoint(nite::JOINT_TORSO), userData.getId() % colorCount);

    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_TORSO), userData.getSkeleton().getJoint(nite::JOINT_LEFT_HIP), userData.getId() % colorCount);
    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_TORSO), userData.getSkeleton().getJoint(nite::JOINT_RIGHT_HIP), userData.getId() % colorCount);

    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_HIP), userData.getSkeleton().getJoint(nite::JOINT_RIGHT_HIP), userData.getId() % colorCount);


    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_HIP), userData.getSkeleton().getJoint(nite::JOINT_LEFT_KNEE), userData.getId() % colorCount);
    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_KNEE), userData.getSkeleton().getJoint(nite::JOINT_LEFT_FOOT), userData.getId() % colorCount);

    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_HIP), userData.getSkeleton().getJoint(nite::JOINT_RIGHT_KNEE), userData.getId() % colorCount);
    DrawLimb(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_KNEE), userData.getSkeleton().getJoint(nite::JOINT_RIGHT_FOOT), userData.getId() % colorCount);
}

void Squircle::computeVideoTexture(nite::UserTrackerFrameRef& userTrackerFrame)
{
    openni::VideoFrameRef depthFrame = userTrackerFrame.getDepthFrame();

    // Texture map init
    if (m_pTexMap == NULL) {
        m_nTexMapX = MIN_CHUNKS_SIZE(depthFrame.getVideoMode().getResolutionX(), TEXTURE_SIZE); // 1024
        m_nTexMapY = MIN_CHUNKS_SIZE(depthFrame.getVideoMode().getResolutionY(), TEXTURE_SIZE); // 512
        m_pTexMap = new openni::RGB888Pixel[m_nTexMapX * m_nTexMapY];
    }



    if (depthFrame.isValid() && g_drawDepth) {
        calculateHistogram(m_pDepthHist, MAX_DEPTH, depthFrame);
        openni::RGB888Pixel* texture = prepareFrameTexture(m_pTexMap, m_nTexMapX, m_nTexMapY, userTrackerFrame);
        loadVideoTexture(texture, m_nTexMapX, m_nTexMapY, m_frameTexture);

        g_nXRes = depthFrame.getVideoMode().getResolutionX();
        g_nYRes = depthFrame.getVideoMode().getResolutionY();
    }
}

openni::RGB888Pixel* Squircle::prepareFrameTexture(openni::RGB888Pixel* texture, unsigned int width, unsigned int height, nite::UserTrackerFrameRef userTrackerFrame)
{
    openni::VideoFrameRef depthFrame = userTrackerFrame.getDepthFrame();
    const nite::UserMap& userLabels = userTrackerFrame.getUserMap();


    memset(texture, 0, width*height*sizeof(openni::RGB888Pixel));

    float factor[3] = {1, 1, 1};
    const nite::UserId* pLabels = userLabels.getPixels();
    const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*) depthFrame.getData();
    openni::RGB888Pixel* pTexRow = texture + depthFrame.getCropOriginY() * width;
    int rowSize = depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel);

    for (int y = 0; y < depthFrame.getHeight(); ++y)
    {
        const openni::DepthPixel* pDepth = pDepthRow;
        openni::RGB888Pixel* pTex = pTexRow + depthFrame.getCropOriginX();

        for (int x = 0; x < depthFrame.getWidth(); ++x, ++pDepth, ++pTex, ++pLabels)
        {
            if (*pDepth != 0){
                if (*pLabels == 0) {
                    if (!g_drawBackground) {
                        factor[0] = factor[1] = factor[2] = 0;
                    } else {
                        factor[0] = Colors[colorCount][0];
                        factor[1] = Colors[colorCount][1];
                        factor[2] = Colors[colorCount][2];
                    }
                } else {
                    factor[0] = Colors[*pLabels % colorCount][0];
                    factor[1] = Colors[*pLabels % colorCount][1];
                    factor[2] = Colors[*pLabels % colorCount][2];
                }
                //					// Add debug lines - every 10cm
                // 					else if ((*pDepth / 10) % 10 == 0)
                // 					{
                // 						factor[0] = factor[2] = 0;
                // 					}

                int nHistValue = m_pDepthHist[*pDepth];
                pTex->r = nHistValue*factor[0];
                pTex->g = nHistValue*factor[1];
                pTex->b = nHistValue*factor[2];

                factor[0] = factor[1] = factor[2] = 1;
            }
        }

        pDepthRow += rowSize;
        pTexRow += width;
    }

    return texture;
}

// Create Texture (overwrite previous)
void Squircle::loadVideoTexture(openni::RGB888Pixel* texture, GLsizei width, GLsizei height, GLuint glTextureId)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}
