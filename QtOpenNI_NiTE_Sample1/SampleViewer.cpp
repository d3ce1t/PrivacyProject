#define TEXTURE_SIZE	512
#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_DEPTH

#define MIN_NUM_CHUNKS(data_size, chunk_size)	((((data_size)-1) / (chunk_size) + 1))
#define MIN_CHUNKS_SIZE(data_size, chunk_size)	(MIN_NUM_CHUNKS(data_size, chunk_size) * (chunk_size))

#include "SampleViewer.h"
#include <QtGui/QScreen>
#include <QApplication>
#include <iostream>
#include <QElapsedTimer>
#include <QKeyEvent>
#include "OniSampleUtilities.h"

bool g_drawSkeleton = true;
bool g_drawCenterOfMass = false;
bool g_drawStatusLabel = true;
bool g_drawBoundingBox = false;
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

/*#ifndef USE_GLES
void glPrintString(void *font, const char *str)
{
    int i,l = (int)strlen(str);

    for(i=0; i<l; i++)
    {
        glutBitmapCharacter(font,*str++);
    }
}
#endif*/

using namespace std;

void SampleViewer::printMessage(const nite::UserData &user, uint64_t ts, const char *msg)
{
    sprintf(g_userStatusLabels[user.getId()],"%s", msg);
    printf("[%08llu] User #%d:\t%s\n",ts, user.getId(),msg);
}

SampleViewer::SampleViewer()
    : m_pTexMap(NULL)
{
    m_pUserTracker = new nite::UserTracker;
    initOpenNI();
}

SampleViewer::~SampleViewer()
{
    cout << "End of application" << endl;

    if (m_pUserTracker != NULL) {
        delete m_pUserTracker;
        m_pUserTracker = NULL;
    }

    nite::NiTE::shutdown();
    m_device.close();
    openni::OpenNI::shutdown();

    if (m_pTexMap != NULL) {
        delete[] m_pTexMap;
        m_pTexMap = NULL;
    }
}

void SampleViewer::initialize()
{
    /*glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);*/

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void SampleViewer::initOpenNI()
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

        // Texture map init
       /* m_nTexMapX = MIN_CHUNKS_SIZE(m_width, TEXTURE_SIZE);
        m_nTexMapY = MIN_CHUNKS_SIZE(m_height, TEXTURE_SIZE);
        m_pTexMap = new openni::RGB888Pixel[m_nTexMapX * m_nTexMapY];*/
    }
    catch (int ex)
    {
        printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
        nite::NiTE::shutdown();
        openni::OpenNI::shutdown();
        throw ex;
    }
}

void SampleViewer::render()
{
    nite::UserTrackerFrameRef userTrackerFrame;
    openni::VideoFrameRef depthFrame;
    nite::Status rc = m_pUserTracker->readFrame(&userTrackerFrame);

    if (rc != nite::STATUS_OK)
    {
        printf("GetNextData failed\n");
        return;
    }

    depthFrame = userTrackerFrame.getDepthFrame();

    if (m_pTexMap == NULL)
    {
        // Texture map init
        m_nTexMapX = MIN_CHUNKS_SIZE(depthFrame.getVideoMode().getResolutionX(), TEXTURE_SIZE);
        m_nTexMapY = MIN_CHUNKS_SIZE(depthFrame.getVideoMode().getResolutionY(), TEXTURE_SIZE);
        m_pTexMap = new openni::RGB888Pixel[m_nTexMapX * m_nTexMapY];
    }

    const nite::UserMap& userLabels = userTrackerFrame.getUserMap();

    glViewport(0, 0, width(), height());
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width(), height(), 0, -1.0, 1.0);

    if (depthFrame.isValid() && g_drawDepth)
    {
        const openni::DepthPixel* pDepth = (const openni::DepthPixel*)depthFrame.getData();
        int width = depthFrame.getWidth();
        int height = depthFrame.getHeight();
        // Calculate the accumulative histogram (the yellow display...)
        memset(m_pDepthHist, 0, MAX_DEPTH*sizeof(float));
        int restOfRow = depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel) - width;

        unsigned int nNumberOfPoints = 0;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x, ++pDepth)
            {
                if (*pDepth != 0)
                {
                    m_pDepthHist[*pDepth]++;
                    nNumberOfPoints++;
                }
            }
            pDepth += restOfRow;
        }
        for (int nIndex=1; nIndex<MAX_DEPTH; nIndex++)
        {
            m_pDepthHist[nIndex] += m_pDepthHist[nIndex-1];
        }
        if (nNumberOfPoints)
        {
            for (int nIndex=1; nIndex<MAX_DEPTH; nIndex++)
            {
                m_pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (m_pDepthHist[nIndex] / nNumberOfPoints)));
            }
        }
    }

    memset(m_pTexMap, 0, m_nTexMapX*m_nTexMapY*sizeof(openni::RGB888Pixel));

    float factor[3] = {1, 1, 1};
    // check if we need to draw depth frame to texture
    if (depthFrame.isValid() && g_drawDepth)
    {
        const nite::UserId* pLabels = userLabels.getPixels();

        const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*)depthFrame.getData();
        openni::RGB888Pixel* pTexRow = m_pTexMap + depthFrame.getCropOriginY() * m_nTexMapX;
        int rowSize = depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel);

        for (int y = 0; y < depthFrame.getHeight(); ++y)
        {
            const openni::DepthPixel* pDepth = pDepthRow;
            openni::RGB888Pixel* pTex = pTexRow + depthFrame.getCropOriginX();

            for (int x = 0; x < depthFrame.getWidth(); ++x, ++pDepth, ++pTex, ++pLabels)
            {
                if (*pDepth != 0)
                {
                    if (*pLabels == 0)
                    {
                        if (!g_drawBackground)
                        {
                            factor[0] = factor[1] = factor[2] = 0;

                        }
                        else
                        {
                            factor[0] = Colors[colorCount][0];
                            factor[1] = Colors[colorCount][1];
                            factor[2] = Colors[colorCount][2];
                        }
                    }
                    else
                    {
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
            pTexRow += m_nTexMapX;
        }
    }

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_nTexMapX, m_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pTexMap);

    // Display the OpenGL texture map
    glColor4f(1,1,1,1);

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    g_nXRes = depthFrame.getVideoMode().getResolutionX();
    g_nYRes = depthFrame.getVideoMode().getResolutionY();

    // upper left
    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    // upper right
    glTexCoord2f((float)g_nXRes/(float)m_nTexMapX, 0);
    glVertex2f(width(), 0);
    // bottom right
    glTexCoord2f((float)g_nXRes/(float)m_nTexMapX, (float)g_nYRes/(float)m_nTexMapY);
    glVertex2f(width(), height());
    // bottom left
    glTexCoord2f(0, (float)g_nYRes/(float)m_nTexMapY);
    glVertex2f(0, height());

    glEnd();
    glDisable(GL_TEXTURE_2D);

    const nite::Array<nite::UserData>& users = userTrackerFrame.getUsers();
    for (int i = 0; i < users.getSize(); ++i)
    {
        const nite::UserData& user = users[i];

        updateUserState(user, userTrackerFrame.getTimestamp());
        if (user.isNew())
        {
            m_pUserTracker->startSkeletonTracking(user.getId());
            m_pUserTracker->startPoseDetection(user.getId(), nite::POSE_CROSSED_HANDS);
        }
        else if (!user.isLost())
        {
            if (g_drawStatusLabel)
            {
                DrawStatusLabel(m_pUserTracker, user);
            }
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

    if (g_drawFrameId)
    {
        DrawFrameId(userTrackerFrame.getFrameIndex());
    }

    /*if (g_generalMessage[0] != '\0')
    {
        char *msg = g_generalMessage;
        glColor3f(1.0f, 0.0f, 0.0f);
        glRasterPos2i(100, 20);

        glPrintString(GLUT_BITMAP_HELVETICA_18, msg);
    }*/
}

void SampleViewer::keyPressEvent(QKeyEvent* ev) {
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

void SampleViewer::updateUserState(const nite::UserData& user, uint64_t ts)
{
    if (user.isNew())
    {
        printMessage(user, ts, "New");
    }
    else if (user.isVisible() && !g_visibleUsers[user.getId()])
        printf("[%08llu] User #%d:\tVisible\n", ts, user.getId());
    else if (!user.isVisible() && g_visibleUsers[user.getId()])
        printf("[%08llu] User #%d:\tOut of Scene\n", ts, user.getId());
    else if (user.isLost())
    {
        printMessage(user, ts, "Lost");
    }
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

void SampleViewer::DrawStatusLabel(nite::UserTracker* pUserTracker, const nite::UserData& user)
{
    int color = user.getId() % colorCount;
    glColor3f(1.0f - Colors[color][0], 1.0f - Colors[color][1], 1.0f - Colors[color][2]);

    float x,y;
    pUserTracker->convertJointCoordinatesToDepth(user.getCenterOfMass().x, user.getCenterOfMass().y, user.getCenterOfMass().z, &x, &y);
    x *= width()/g_nXRes;
    y *= height()/g_nYRes;
    char *msg = g_userStatusLabels[user.getId()];
    glRasterPos2i(x-((strlen(msg)/2)*8),y);
    //glPrintString(GLUT_BITMAP_HELVETICA_18, msg);
}

void SampleViewer::DrawFrameId(int frameId)
{
    char buffer[80] = "";
    sprintf(buffer, "%d", frameId);
    glColor3f(1.0f, 0.0f, 0.0f);
    glRasterPos2i(20, 20);
    //glPrintString(GLUT_BITMAP_HELVETICA_18, buffer);
}

void SampleViewer::DrawCenterOfMass(nite::UserTracker* pUserTracker, const nite::UserData& user)
{
    glColor3f(1.0f, 1.0f, 1.0f);

    float coordinates[3] = {0};

    pUserTracker->convertJointCoordinatesToDepth(user.getCenterOfMass().x, user.getCenterOfMass().y, user.getCenterOfMass().z, &coordinates[0], &coordinates[1]);

    coordinates[0] *= width()/g_nXRes;
    coordinates[1] *= height()/g_nYRes;
    glPointSize(8);
    glVertexPointer(3, GL_FLOAT, 0, coordinates);
    glDrawArrays(GL_POINTS, 0, 1);

}

void SampleViewer::DrawBoundingBox(const nite::UserData& user)
{
    glColor3f(1.0f, 1.0f, 1.0f);

    float coordinates[] =
    {
        user.getBoundingBox().max.x, user.getBoundingBox().max.y, 0,
        user.getBoundingBox().max.x, user.getBoundingBox().min.y, 0,
        user.getBoundingBox().min.x, user.getBoundingBox().min.y, 0,
        user.getBoundingBox().min.x, user.getBoundingBox().max.y, 0,
    };
    coordinates[0]  *= width()/g_nXRes;
    coordinates[1]  *= height()/g_nYRes;
    coordinates[3]  *= width()/g_nXRes;
    coordinates[4]  *= height()/g_nYRes;
    coordinates[6]  *= width()/g_nXRes;
    coordinates[7]  *= height()/g_nYRes;
    coordinates[9]  *= width()/g_nXRes;
    coordinates[10] *= height()/g_nYRes;

    glPointSize(2);
    glVertexPointer(3, GL_FLOAT, 0, coordinates);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

}

void SampleViewer::DrawLimb(nite::UserTracker* pUserTracker, const nite::SkeletonJoint& joint1, const nite::SkeletonJoint& joint2, int color)
{
    float coordinates[6] = {0};
    pUserTracker->convertJointCoordinatesToDepth(joint1.getPosition().x, joint1.getPosition().y, joint1.getPosition().z, &coordinates[0], &coordinates[1]);
    pUserTracker->convertJointCoordinatesToDepth(joint2.getPosition().x, joint2.getPosition().y, joint2.getPosition().z, &coordinates[3], &coordinates[4]);

    coordinates[0] *= width()/g_nXRes;
    coordinates[1] *= height()/g_nYRes;
    coordinates[3] *= width()/g_nXRes;
    coordinates[4] *= height()/g_nYRes;

    if (joint1.getPositionConfidence() == 1 && joint2.getPositionConfidence() == 1)
    {
        glColor3f(1.0f - Colors[color][0], 1.0f - Colors[color][1], 1.0f - Colors[color][2]);
    }
    else if (joint1.getPositionConfidence() < 0.5f || joint2.getPositionConfidence() < 0.5f)
    {
        return;
    }
    else
    {
        glColor3f(.5, .5, .5);
    }
    glPointSize(2);
    glVertexPointer(3, GL_FLOAT, 0, coordinates);
    glDrawArrays(GL_LINES, 0, 2);

    glPointSize(10);
    if (joint1.getPositionConfidence() == 1)
    {
        glColor3f(1.0f - Colors[color][0], 1.0f - Colors[color][1], 1.0f - Colors[color][2]);
    }
    else
    {
        glColor3f(.5, .5, .5);
    }
    glVertexPointer(3, GL_FLOAT, 0, coordinates);
    glDrawArrays(GL_POINTS, 0, 1);

    if (joint2.getPositionConfidence() == 1)
    {
        glColor3f(1.0f - Colors[color][0], 1.0f - Colors[color][1], 1.0f - Colors[color][2]);
    }
    else
    {
        glColor3f(.5, .5, .5);
    }
    glVertexPointer(3, GL_FLOAT, 0, coordinates+3);
    glDrawArrays(GL_POINTS, 0, 1);
}

void SampleViewer::DrawSkeleton(nite::UserTracker* pUserTracker, const nite::UserData& userData)
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
