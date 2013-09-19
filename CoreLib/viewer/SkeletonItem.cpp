#include "SkeletonItem.h"
#include "viewer/ScenePainter.h"

namespace dai {

#define RedColor   QVector3D(1.0, 0, 0)
#define GreenColor QVector3D(0, 1.0, 0)
#define BlueColor  QVector3D(0, 0, 1.0)
#define BlackColor QVector3D(0, 0, 0)
#define YellowColor QVector3D(1.0, 1.0, 0);

QVector3D SkeletonItem::staticJointsColor[20] = {
    BlueColor, // JOINT_HEAD
    BlackColor, // JOINT_CENTER_SHOULDER
    RedColor,   // JOINT_LEFT_SHOULDER
    GreenColor, // JOINT_RIGHT_SHOULDER
    RedColor,   // JOINT_LEFT_ELBOW
    GreenColor, // JOINT_RIGHT_ELBOW
    RedColor,   // JOINT_LEFT_WRIST
    GreenColor, // JOINT_RIGHT_WRIST
    RedColor,   // JOINT_LEFT_HAND
    GreenColor, // JOINT_RIGHT_HAND
    BlackColor, // JOINT_SPINE
    BlackColor, // JOINT_CENTER_HIP
    RedColor,   // JOINT_LEFT_HIP
    GreenColor, // JOINT_RIGHT_HIP
    RedColor,   // JOINT_LEFT_KNEE
    GreenColor, // JOINT_RIGHT_KNEE
    RedColor,   // JOINT_LEFT_ANKLE
    GreenColor, // JOINT_RIGHT_ANKLE
    RedColor,   // JOINT_LEFT_FOOT
    GreenColor // JOINT_RIGHT_FOOT
};

SkeletonItem::SkeletonItem()
    : SceneItem(ITEM_SKELETON)
{
}

void SkeletonItem::setSkeleton(shared_ptr<SkeletonFrame> skeleton)
{
    m_frame = static_pointer_cast<SkeletonFrame>(skeleton);
}

void SkeletonItem::initialise()
{
    // Load, compile and link the shader program
    prepareShaderProgram();

    // Set the clear color
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void SkeletonItem::render(int pass)
{
    if (m_frame == nullptr)
        return;

    m_shaderProgram->bind();

    foreach (int userId, m_frame->getAllUsersId())
    {
        const dai::Skeleton& skeleton = *(m_frame->getSkeleton(userId));
        const dai::Skeleton::SkeletonLimb* limbMap = skeleton.getLimbsMap();

        for (int i=0; i<skeleton.getLimbsCount(); ++i) {
            drawLimb( skeleton.getJoint(limbMap[i].joint1), skeleton.getJoint(limbMap[i].joint2) );
        }
    }

    m_shaderProgram->release();
}

void SkeletonItem::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/glsl/skeleton.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/glsl/skeleton.fsh");
    m_shaderProgram->bindAttributeLocation("posAttr", 0);
    m_shaderProgram->bindAttributeLocation("colAttr", 1);

    m_shaderProgram->link();

    m_posAttr = m_shaderProgram->attributeLocation("posAttr");
    m_colorAttr = m_shaderProgram->attributeLocation("colAttr");
    m_pointSize = m_shaderProgram->uniformLocation("sizeAttr");
    m_perspectiveMatrix = m_shaderProgram->uniformLocation("perspectiveMatrix");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, scene()->getMatrix() );
    m_shaderProgram->setUniformValue(m_pointSize, 2.0f);
    m_shaderProgram->release();
}

void SkeletonItem::drawLimb(const dai::SkeletonJoint& joint1, const dai::SkeletonJoint& joint2)
{
    float coordinates[] = {
        (float) joint1.getPosition().x(), (float) -joint1.getPosition().y(), (float) -joint1.getPosition().z(),
        (float) joint2.getPosition().x(), (float) -joint2.getPosition().y(), (float) -joint2.getPosition().z()
    };

    float coorColours[] = {
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0
    };

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glDisable(GL_DEPTH_TEST);

     // Draw Line from joint1 to joint2
    m_shaderProgram->setAttributeArray(m_posAttr, coordinates, 3);
    m_shaderProgram->setAttributeArray(m_colorAttr, coorColours, 3);
    m_shaderProgram->setUniformValue(m_pointSize, 8.0f);
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, scene()->getMatrix());
    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->enableAttributeArray(m_colorAttr);
    glDrawArrays(GL_LINES, m_posAttr, 2);

    // Draw Joint 1
    coorColours[0] = staticJointsColor[joint1.getType()].x();
    coorColours[1] = staticJointsColor[joint1.getType()].y();
    coorColours[2] = staticJointsColor[joint1.getType()].z();
    m_shaderProgram->setAttributeArray(m_colorAttr, coorColours, 3);
    glDrawArrays(GL_POINTS, m_posAttr, 1);

    // Draw Joint 2
    coorColours[0] = staticJointsColor[joint2.getType()].x();
    coorColours[1] = staticJointsColor[joint2.getType()].y();
    coorColours[2] = staticJointsColor[joint2.getType()].z();
    m_shaderProgram->setAttributeArray(m_colorAttr, coorColours, 3);
    m_shaderProgram->setAttributeArray(m_posAttr, coordinates + 3, 3);
    glDrawArrays(GL_POINTS, m_posAttr, 1);

    // Release
    m_shaderProgram->disableAttributeArray(m_colorAttr);
    m_shaderProgram->disableAttributeArray(m_posAttr);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

} // End Namespace
