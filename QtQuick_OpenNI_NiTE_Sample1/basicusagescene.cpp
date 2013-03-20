#include "basicusagescene.h"
#include <QOpenGLShaderProgram>


BasicUsageScene::BasicUsageScene()
    : colorCount(3)
{
    colors[0] = QVector3D(1, 0, 0);
    colors[1] = QVector3D(0, 1, 0);
    colors[2] = QVector3D(0, 0, 1);
    colors[3] = QVector3D(1, 1, 1);

    m_shaderProgram = NULL;
    m_pUserTracker = NULL;

    g_drawSkeleton = true;
    g_drawCenterOfMass = true;
    g_drawStatusLabel = false;
    g_drawBoundingBox = true;
    g_drawFrameId = false;
}

BasicUsageScene::~BasicUsageScene()
{
    if (m_shaderProgram != NULL)
    {
        delete m_shaderProgram;
        m_shaderProgram = NULL;
    }

    if (m_pUserTracker) {
        m_pUserTracker = NULL;
    }
}

void BasicUsageScene::initialise()
{
    // Load, compile and link the shader program
    prepareShaderProgram();

    // Set the clear color
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void BasicUsageScene::update( float t )
{
    Q_UNUSED(t);
}

void BasicUsageScene::render()
{
    /* if (g_drawStatusLabel)
     {
         //DrawStatusLabel(m_pUserTracker, user);
     }*/
     if (g_drawCenterOfMass)
     {
         DrawCenterOfMass(m_pUserTracker, m_user);
     }
     if (g_drawBoundingBox)
     {
         DrawBoundingBox(m_user);
     }

     if (m_user.getSkeleton().getState() == nite::SKELETON_TRACKED && g_drawSkeleton)
     {
         DrawSkeleton(m_pUserTracker, m_user);
     }
}

void BasicUsageScene::resize( float w, float h )
{
    m_width = w;
    m_height = h;
}

void BasicUsageScene::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/simpleVertex.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/simpleFragment.fsh");
    m_shaderProgram->bindAttributeLocation("posAttr", 0);
    m_shaderProgram->bindAttributeLocation("colAttr", 1);

    m_shaderProgram->link();

    m_posAttr = m_shaderProgram->attributeLocation("posAttr");
    m_colorAttr = m_shaderProgram->attributeLocation("colAttr");
    m_pointSize = m_shaderProgram->uniformLocation("sizeAttr");
    m_perspectiveMatrix = m_shaderProgram->uniformLocation("perspectiveMatrix");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);
    m_shaderProgram->setUniformValue(m_pointSize, 2.0f);
    m_shaderProgram->release();
}

void BasicUsageScene::setMatrix(QMatrix4x4& matrix) {
    this->m_matrix = matrix;
}

void BasicUsageScene::setUser(const nite::UserData& user)
{
    this->m_user = user;
}

void BasicUsageScene::setUserTrackerPointer(nite::UserTracker* pUserTracker)
{
    m_pUserTracker = pUserTracker;
}

void BasicUsageScene::setNativeResolution(int width, int height)
{
    m_nativeWidth = width;
    m_nativeHeight = height;
}

void BasicUsageScene::DrawCenterOfMass(nite::UserTracker* pUserTracker, const nite::UserData& user)
{
    float coordinates[2] = {0};
    const GLfloat centerColour[] = {1.0f, 1.0f, 1.0f};

    pUserTracker->convertJointCoordinatesToDepth(user.getCenterOfMass().x, user.getCenterOfMass().y, user.getCenterOfMass().z, &coordinates[0], &coordinates[1]);

    coordinates[0] *= m_width/m_nativeWidth;
    coordinates[1] *= m_height/m_nativeHeight;

    // Draw
    glPushMatrix();

    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);


    m_shaderProgram->bind();
    m_shaderProgram->setAttributeArray(m_posAttr, coordinates, 2);
    m_shaderProgram->setAttributeArray(m_colorAttr, centerColour, 3);
    m_shaderProgram->setUniformValue(m_pointSize, 8.0f);

    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->enableAttributeArray(m_colorAttr);

    glDrawArrays(GL_POINTS, m_posAttr, 1);

    m_shaderProgram->disableAttributeArray(m_colorAttr);
    m_shaderProgram->disableAttributeArray(m_posAttr);
    m_shaderProgram->release();

    glPopMatrix();

    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}
void BasicUsageScene::DrawSkeleton(nite::UserTracker* pUserTracker, const nite::UserData& userData)
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

void BasicUsageScene::DrawBoundingBox(const nite::UserData& user)
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

    coordinates[0]  *= m_width/m_nativeWidth;
    coordinates[1]  *= m_height/m_nativeHeight;
    coordinates[2]  *= m_width/m_nativeWidth;
    coordinates[3]  *= m_height/m_nativeHeight;
    coordinates[4]  *= m_width/m_nativeWidth;
    coordinates[5]  *= m_height/m_nativeHeight;
    coordinates[6]  *= m_width/m_nativeWidth;
    coordinates[7]  *= m_height/m_nativeHeight;

    glPushMatrix();
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    m_shaderProgram->bind();
    m_shaderProgram->setAttributeArray(m_posAttr, coordinates, 2);
    m_shaderProgram->setAttributeArray(m_colorAttr, coorColours, 3);
    m_shaderProgram->setUniformValue(m_pointSize, 2.0f);
    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->enableAttributeArray(m_colorAttr);

    glDrawArrays(GL_LINE_LOOP, m_posAttr, 4);

    m_shaderProgram->disableAttributeArray(m_colorAttr);
    m_shaderProgram->disableAttributeArray(m_posAttr);
    m_shaderProgram->release();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glPopMatrix();
}

void BasicUsageScene::DrawLimb(nite::UserTracker* pUserTracker, const nite::SkeletonJoint& joint1, const nite::SkeletonJoint& joint2, int color)
{
    if (joint1.getPositionConfidence() < 0.5f || joint2.getPositionConfidence() < 0.5f)
        return;

    float coordinates[4] = {0};
    pUserTracker->convertJointCoordinatesToDepth(joint1.getPosition().x, joint1.getPosition().y, joint1.getPosition().z, &coordinates[0], &coordinates[1]);
    pUserTracker->convertJointCoordinatesToDepth(joint2.getPosition().x, joint2.getPosition().y, joint2.getPosition().z, &coordinates[2], &coordinates[3]);

    coordinates[0] *= m_width/m_nativeWidth;
    coordinates[1] *= m_height/m_nativeHeight;
    coordinates[2] *= m_width/m_nativeWidth;
    coordinates[3] *= m_height/m_nativeHeight;

    float factor = (joint1.getPositionConfidence() + joint2.getPositionConfidence()) / 2.0f;


    float coorColours[3] = {colors[color].x() * factor,
                            colors[color].y() * factor,
                            colors[color].z() * factor};

    glPushMatrix();
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    // Bind Shader
    m_shaderProgram->bind();

     // Draw Line from joint1 to joint2
    m_shaderProgram->setAttributeArray(m_posAttr, coordinates, 2);
    m_shaderProgram->setAttributeArray(m_colorAttr, coorColours, 3);
    m_shaderProgram->setUniformValue(m_pointSize, 3.0f);
    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->enableAttributeArray(m_colorAttr);
    glDrawArrays(GL_LINES, m_posAttr, 2);

    // Draw point for joint1
    coorColours[0] = colors[color].x() * joint1.getPositionConfidence();
    coorColours[1] = colors[color].y() * joint1.getPositionConfidence();
    coorColours[2] = colors[color].z() * joint1.getPositionConfidence();
    m_shaderProgram->setUniformValue(m_pointSize, 10.0f);
    m_shaderProgram->setAttributeArray(m_colorAttr, coorColours, 3);
    glDrawArrays(GL_POINTS, m_posAttr, 1);

    // Draw point for joint2
    coorColours[0] = colors[color].x() * joint2.getPositionConfidence();
    coorColours[1] = colors[color].y() * joint2.getPositionConfidence();
    coorColours[2] = colors[color].z() * joint2.getPositionConfidence();
    m_shaderProgram->setAttributeArray(m_colorAttr, coorColours, 3);
    m_shaderProgram->setAttributeArray(m_posAttr, coordinates+2, 2);
    glDrawArrays(GL_POINTS, m_posAttr, 1);

    // Release
    m_shaderProgram->disableAttributeArray(m_colorAttr);
    m_shaderProgram->disableAttributeArray(m_posAttr);
    m_shaderProgram->release();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glPopMatrix();
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

void BasicUsageScene::setDrawSkeletonFlag(bool value)
{
    g_drawSkeleton = value;
    emit changeOfStatus();
}

void BasicUsageScene::setDrawCenterOfMassFlag(bool value)
{
    g_drawCenterOfMass = value;
    emit changeOfStatus();
}

void BasicUsageScene::setDrawBoundingBoxFlag(bool value)
{
    g_drawBoundingBox = value;
    emit changeOfStatus();
}
