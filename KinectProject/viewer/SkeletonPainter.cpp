#include "SkeletonPainter.h"
#include <QOpenGLShaderProgram>
#include "../types/Skeleton.h"
#include "../dataset/DataInstance.h"
#include <QMetaEnum>
#include "CustomItem.h"
#include <cmath>

namespace dai {

SkeletonPainter::SkeletonPainter(StreamInstance *instance, InstanceViewer *parent)
    : ViewerPainter(instance, parent)
{
    if (instance->getType() != StreamInstance::Skeleton)
        throw 1;

    m_shaderProgram = NULL;
    m_isFrameAvailable = false;

    // Setup Joints Model
    m_joints_model.setRowCount(20);
    m_joints_model.setColumnCount(3);
    m_joints_table_view.setWindowTitle("Joints info for " + instance->getTitle());
    m_joints_table_view.setModel(&m_joints_model);
    m_joints_table_view.setMinimumSize(460, 630);

    QStringList list;
    list << "pos X" << "pos Y" << "pos Z";
    m_joints_model.setHorizontalHeaderLabels(list);

    for (int i=0; i<20; ++i) {
        QStandardItem *item = new QStandardItem;
        m_joints_model.setVerticalHeaderItem(i, item);
        for (int j=0; j<3; ++j) {
            CustomItem *item = new CustomItem;
            m_joints_model.setItem(i, j, item);
        }
    }

    // Setup Distances model
    m_distances_model.setRowCount(20);
    m_distances_model.setColumnCount(20);
    m_distances_table_view.setWindowTitle("Distances info for " + instance->getTitle());
    m_distances_table_view.setModel(&m_distances_model);
    m_distances_table_view.setMinimumSize(600, 640);

    const QMetaObject &skeletonJointMetaObject = dai::SkeletonJoint::staticMetaObject;
    int index = skeletonJointMetaObject.indexOfEnumerator("JointType"); // watch out during refactorings
    QMetaEnum metaEnum = skeletonJointMetaObject.enumerator(index);

    list.clear();

    for (int i=0; i<20; ++i) {
        QString name(metaEnum.valueToKey(i));
        list << name.mid(6);
    }

    m_distances_model.setHorizontalHeaderLabels(list);

    for (int i=0; i<20; ++i) {
        QStandardItem *item = new QStandardItem;
        item->setText(list.at(i));
        m_distances_model.setVerticalHeaderItem(i, item);
        for (int j=0; j<20; ++j) {
            CustomItem *item = new CustomItem;
            m_distances_model.setItem(i, j, item);
        }
    }

    // Setup Quaternions Model
    m_quaternions_model.setRowCount(20);
    m_quaternions_model.setColumnCount(5);
    m_quaternions_table_view.setWindowTitle("Quaternions info for " + instance->getTitle());
    m_quaternions_table_view.setModel(&m_quaternions_model);
    m_quaternions_table_view.setMinimumSize(510, 545);

    const QMetaObject &quaternionMetaObject = dai::Quaternion::staticMetaObject;
    index = quaternionMetaObject.indexOfEnumerator("QuaternionType");
    metaEnum = quaternionMetaObject.enumerator(index);

    list.clear();
    list << "Tensor" <<  "pos X" << "pos Y" << "pos Z" << "angle";
    m_quaternions_model.setHorizontalHeaderLabels(list);

    list.clear();

    for (int i=0; i<20; ++i) {
        QString name(metaEnum.valueToKey(i));
        list << name.mid(11);
    }

    for (int i=0; i<20; ++i) {
        QStandardItem *item = new QStandardItem;
        item->setText(list.at(i));
        m_quaternions_model.setVerticalHeaderItem(i, item);
        for (int j=0; j<5; ++j) {
            QStandardItem *item = new QStandardItem;
            m_quaternions_model.setItem(i, j, item);
        }
    }

    // Show Tables
    //m_joints_table_view.setWindowOpacity(0.85);
    m_joints_table_view.show();
    //m_distances_table_view.setWindowOpacity(0.85);
    m_distances_table_view.show();
    //m_quaternions_table_view.setWindowOpacity(0.85);
    m_quaternions_table_view.show();
}

SkeletonPainter::~SkeletonPainter()
{
    if (m_shaderProgram != NULL)
    {
        delete m_shaderProgram;
        m_shaderProgram = NULL;
    }

    m_joints_model.clear();
    m_joints_table_view.close();
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
        loadModels();
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

void SkeletonPainter::loadModels()
{
    // Joints Model
    const QList<dai::SkeletonJoint*>& joints = m_skeleton.getAllJoints();
    QListIterator<dai::SkeletonJoint*> it(joints);
    int row = 0;

    const QMetaObject &mo = dai::SkeletonJoint::staticMetaObject;
    int index = mo.indexOfEnumerator("JointType"); // watch out during refactorings
    QMetaEnum metaEnum = mo.enumerator(index);

    while (it.hasNext())
    {        
        dai::SkeletonJoint* joint = it.next();

        CustomItem* itemX = dynamic_cast<CustomItem*>(m_joints_model.item(row, 0));
        CustomItem* itemY = dynamic_cast<CustomItem*>(m_joints_model.item(row, 1));
        CustomItem* itemZ = dynamic_cast<CustomItem*>(m_joints_model.item(row, 2));

        QString name(metaEnum.valueToKey(joint->getType()));
        name = name.mid(6);

        (m_joints_model.verticalHeaderItem(row))->setText(name);
        itemX->setNumber(joint->getPosition().x());
        itemY->setNumber(joint->getPosition().y());
        itemZ->setNumber(joint->getPosition().z());

        row++;
    }

    // Distances Model
    for (int i=0; i<20; ++i) {
        SkeletonJoint::JointType joint1 = (SkeletonJoint::JointType) i;
        for (int j=0; j<20; ++j) {
            SkeletonJoint::JointType joint2 = (SkeletonJoint::JointType) j;
            float distance = 0;
            if (j < i) {
                distance = Point3f::distance(m_skeleton.getJoint(joint1).getPosition(), m_skeleton.getJoint(joint2).getPosition(), Point3f::DISTANCE_EUCLIDEAN);
            } else if (j > i) {
                distance = -1;
            }
            CustomItem* item = dynamic_cast<CustomItem*>(m_distances_model.item(i, j));
            item->setNumber(distance);
        }
    }

    m_distances_table_view.resizeColumnsToContents();

    const QMetaObject &quaternionMetaObject = dai::Quaternion::staticMetaObject;
    index = quaternionMetaObject.indexOfEnumerator("QuaternionType");
    metaEnum = quaternionMetaObject.enumerator(index);

    // Quaternions Model
    for (int i=0; i<20; ++i)
    {
        const Quaternion& quaternion = m_skeleton.getQuaternion( (Quaternion::QuaternionType) i);

        QStandardItem* itemTensor = m_quaternions_model.item(i, 0);
        QStandardItem* itemX = m_quaternions_model.item(i, 1);
        QStandardItem* itemY = m_quaternions_model.item(i, 2);
        QStandardItem* itemZ = m_quaternions_model.item(i, 3);
        QStandardItem* itemAngle = m_quaternions_model.item(i, 4);

        float angle = quaternion.getAngle()*180/M_PI;
        float lastAngle = itemAngle->text().toFloat();
        float diff = 0;

        if (angle < lastAngle && lastAngle != 0) {
            diff = 1 - (angle / lastAngle);
        }
        else if (angle != 0) {
            diff = 1 - (lastAngle / angle);
        }

        itemTensor->setText(QString::number(quaternion.scalar()));
        itemX->setText(QString::number(quaternion.vector().x()));
        itemY->setText(QString::number(quaternion.vector().y()));
        itemZ->setText(QString::number(quaternion.vector().z()));
        itemAngle->setText(QString::number(angle));

        QBrush brush;
        brush.setStyle(Qt::SolidPattern);
        QColor color(255, 255, 255);
        color.setRgb(255, 0, 0);
        color.setAlphaF(colorIntensity(diff));

       /* if (angle > lastAngle + 0.3) {
            qDebug() << angle << lastAngle << diff;
        }*/

        brush.setColor(color);
        itemTensor->setBackground(brush);
        itemX->setBackground(brush);
        itemY->setBackground(brush);
        itemZ->setBackground(brush);
        itemAngle->setBackground(brush);
    }
}

float SkeletonPainter::colorIntensity(float x)
{
    const double b = 2.30258509299405;
    const double max = 2.39789527279837;
    // max =  1.79175946922805
    // b =  2.99573227355399
    return (log(100*(x+0.1)) - b)/max;
}

void SkeletonPainter::render()
{
    if (!m_isFrameAvailable)
        return;

    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_HEAD), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_CENTER_SHOULDER));
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_CENTER_SHOULDER), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_SPINE));
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_SPINE), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_CENTER_HIP));

    // Left Part
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_CENTER_SHOULDER), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_SHOULDER));
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_SHOULDER), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_ELBOW));
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_ELBOW), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_WRIST));
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_WRIST), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_HAND));

    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_CENTER_HIP), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_HIP));
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_HIP), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_KNEE));
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_KNEE), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_ANKLE));
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_ANKLE), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_FOOT));

    // Right Part
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_CENTER_SHOULDER), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_SHOULDER));
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_SHOULDER), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_ELBOW));
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_ELBOW), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_WRIST));
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_WRIST), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_HAND));

    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_CENTER_HIP), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_HIP));
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_HIP), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_KNEE));
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_KNEE), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_ANKLE));
    drawLimb(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_ANKLE), m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_FOOT));

    QVector3D red(1.0, 0.0, 0.0);
    QVector3D green(0.0, 1.0, 0.0);
    QVector3D black(0.0, 0.0, 0.0);

    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_HEAD), black);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_CENTER_SHOULDER), black);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_SPINE), black);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_CENTER_HIP), black);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_SHOULDER), red);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_ELBOW), red);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_WRIST), red);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_HAND), red);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_HIP), red);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_KNEE), red);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_ANKLE), red);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_LEFT_FOOT), red);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_SHOULDER), green);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_ELBOW), green);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_WRIST), green);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_HAND), green);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_HIP), green);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_KNEE), green);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_ANKLE), green);
    drawJoint(m_skeleton.getNormalisedJoint(dai::SkeletonJoint::JOINT_RIGHT_FOOT), green);

    //drawQuaternions();
}

/*void SkeletonPainter::drawQuaternions()
{
    for (int i=0; i<MAX_JOINTS-3; ++i)
    {
        const Quaternion& quaternion = m_skeleton.getQuaternion( (Quaternion::QuaternionType) i);

        SkeletonVector::VectorType v1_offset;
        SkeletonVector::VectorType v2_offset;
        m_skeleton.mapQuaternionToVectors( (Quaternion::QuaternionType) i, &v1_offset, &v2_offset);

        const SkeletonVector& v1 = m_skeleton.getVector(v1_offset);
        const SkeletonVector& v2 = m_skeleton.getVector(v2_offset);
        Point3f point;

        // Search origin
        if (v1.joint1().getType() == v2.joint1().getType()) {
            point = v1.joint1().getPosition();
        } else if (v1.joint1().getType() == v2.joint2().getType()) {
            point = v1.joint1().getPosition();
        } else if (v1.joint2().getType() == v2.joint1().getType()) {
            point = v1.joint1().getPosition();
        } else if (v2.joint2().getType() == v2.joint2().getType()) {
            point = v1.joint1().getPosition();
        }

        float coordinates[] = {
            point.x() + quaternion.vector().x(), point.y() + quaternion.vector().y(), - (point.z() + quaternion.vector().z()),
            -(point.x() + quaternion.vector().x()), -(point.y() + quaternion.vector().y()), point.z() + quaternion.vector().z(),
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

        // Release
        m_shaderProgram->disableAttributeArray(m_colorAttr);
        m_shaderProgram->disableAttributeArray(m_posAttr);
        m_shaderProgram->release();

        glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
    }
}*/

void SkeletonPainter::resize( float w, float h )
{
    m_width = w;
    m_height = h;
}

void SkeletonPainter::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/glsl/simpleVertex.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/glsl/simpleFragment.fsh");
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
