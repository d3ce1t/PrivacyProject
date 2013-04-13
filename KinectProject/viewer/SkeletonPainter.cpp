#include "SkeletonPainter.h"
#include <QOpenGLShaderProgram>
#include "types/Skeleton.h"
#include "dataset/DataInstance.h"

namespace dai {

SkeletonPainter::SkeletonPainter(DataInstance *instance)
    : ViewerPainter(instance)
{
    if (instance->getMetadata().getType() != InstanceInfo::Skeleton)
        throw 1;

    m_shaderProgram = NULL;
    m_isFrameAvailable = false;
}

SkeletonPainter::~SkeletonPainter()
{
    if (m_shaderProgram != NULL)
    {
        delete m_shaderProgram;
        m_shaderProgram = NULL;
    }
}

void SkeletonPainter::initialise()
{
    // Load, compile and link the shader program
    prepareShaderProgram();

    // Set the clear color
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

bool SkeletonPainter::prepareNext()
{
    bool result = false;

    if (m_instance != NULL && m_instance->hasNext())
    {
        const dai::DataFrame& frame = m_instance->nextFrame();
        const dai::Skeleton skeletonFrame = static_cast<const dai::Skeleton&>(frame);
        // FIX: Frame copy. And I'm not implementing operator=. I should not copy.
        m_skeleton = skeletonFrame;
        m_isFrameAvailable = true;
        result = true;
    }
    else if (m_instance != NULL)
    {
        m_instance->close();
        qDebug() << "Closed";
    }

    return result;
}

void SkeletonPainter::render()
{
    if (!m_isFrameAvailable)
        return;

    /*QVector3D maxValue = Skeleton::maxValue(m_skeleton);
    QVector3D minValue = Skeleton::minValue(m_skeleton);*/

    //qDebug() << "Max: " << maxValue;
    //qDebug() << "Min: " << minValue;

    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_HEAD), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_CENTER_SHOULDER));
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_CENTER_SHOULDER), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_SPINE));
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_SPINE), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_CENTER_HIP));

    // Left Part
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_CENTER_SHOULDER), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_SHOULDER));
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_SHOULDER), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_ELBOW));
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_ELBOW), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_WRIST));
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_WRIST), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_HAND));

    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_CENTER_HIP), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_HIP));
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_HIP), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_KNEE));
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_KNEE), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_ANKLE));
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_ANKLE), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_FOOT));

    // Right Part
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_CENTER_SHOULDER), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_SHOULDER));
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_SHOULDER), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_ELBOW));
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_ELBOW), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_WRIST));
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_WRIST), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_HAND));

    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_CENTER_HIP), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_HIP));
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_HIP), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_KNEE));
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_KNEE), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_ANKLE));
    drawLimb(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_ANKLE), m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_FOOT));

    /*QVector3D red(1.0, 0.0, 0.0);
    QVector3D green(0.0, 1.0, 0.0);
    QVector3D blue(0.0, 0.0, 1.0);
    QVector3D black(0.0, 0.0, 0.0);
    QVector3D yellow(1.0, 1.0, 0.0);
    QVector3D other(0.0, 1.0, 1.0);

    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_HEAD), black);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_CENTER_SHOULDER), black);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_SPINE), black);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_CENTER_HIP), black);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_SHOULDER), red);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_ELBOW), red);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_WRIST), yellow);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_HAND), blue);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_HIP), red);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_KNEE), red);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_ANKLE), red);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_LEFT_FOOT), red);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_SHOULDER), green);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_ELBOW), green);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_WRIST), green);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_HAND), green);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_HIP), green);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_KNEE), green);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_ANKLE), green);
    drawJoint(m_skeleton.getJoint(dai::SkeletonJoint::JOINT_RIGHT_FOOT), green);*/
}

void SkeletonPainter::resize( float w, float h )
{
    m_width = w;
    m_height = h;
}

void SkeletonPainter::prepareShaderProgram()
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

void SkeletonPainter::drawLimb(const dai::SkeletonJoint& joint1, const dai::SkeletonJoint& joint2)
{
    float coordinates[] = {
        joint1.getPosition().x(), joint1.getPosition().y(), -joint1.getPosition().z(),
        joint2.getPosition().x(), joint2.getPosition().y(), -joint2.getPosition().z()
    };

    float coorColours[] = {
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0
    };

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glDisable(GL_DEPTH_TEST);

    // Bind Shader
    m_shaderProgram->bind();

     // Draw Line from joint1 to joint2
    m_shaderProgram->setAttributeArray(m_posAttr, coordinates, 3);
    m_shaderProgram->setAttributeArray(m_colorAttr, coorColours, 3);
    m_shaderProgram->setUniformValue(m_pointSize, 8.0f);
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);
    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->enableAttributeArray(m_colorAttr);
    glDrawArrays(GL_LINES, m_posAttr, 2);

    // Draw point for joint1
    m_shaderProgram->setUniformValue(m_pointSize, 10.0f);
    m_shaderProgram->setAttributeArray(m_colorAttr, coorColours, 3);
    glDrawArrays(GL_POINTS, m_posAttr, 1);

    // Draw point for joint2
    m_shaderProgram->setAttributeArray(m_colorAttr, coorColours, 3);
    m_shaderProgram->setAttributeArray(m_posAttr, coordinates+3, 3);
    glDrawArrays(GL_POINTS, m_posAttr, 1);

    // Release
    m_shaderProgram->disableAttributeArray(m_colorAttr);
    m_shaderProgram->disableAttributeArray(m_posAttr);
    m_shaderProgram->release();

    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

void SkeletonPainter::drawJoint(const dai::SkeletonJoint& joint, const QVector3D &color)
{
    float coordinates[] = {
        joint.getPosition().x(), joint.getPosition().y(), -joint.getPosition().z()
    };

    float coorColours[] = {
        color.x(), color.y(), color.z()
    };

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    // Bind Shader
    m_shaderProgram->bind();

     // Draw Line from joint1 to joint2
    m_shaderProgram->setAttributeArray(m_posAttr, coordinates, 3);
    m_shaderProgram->setAttributeArray(m_colorAttr, coorColours, 3);
    m_shaderProgram->setUniformValue(m_pointSize, 8.0f);
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);
    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->enableAttributeArray(m_colorAttr);
    glDrawArrays(GL_POINTS, m_posAttr, 1);

    // Release
    m_shaderProgram->disableAttributeArray(m_colorAttr);
    m_shaderProgram->disableAttributeArray(m_posAttr);
    m_shaderProgram->release();

    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

} // End Namespace
