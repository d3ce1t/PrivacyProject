#include "SkeletonPainter.h"
#include <QOpenGLShaderProgram>
#include "types/Skeleton.h"
#include "dataset/DataInstance.h"
#include <QMetaEnum>

namespace dai {

SkeletonPainter::SkeletonPainter(DataInstance *instance, InstanceViewer *parent)
    : ViewerPainter(instance, parent)
{
    if (instance->getMetadata().getType() != InstanceInfo::Skeleton)
        throw 1;

    m_shaderProgram = NULL;
    m_isFrameAvailable = false;

    // Setup Joints Model
    m_joints_model.setRowCount(20);
    m_joints_model.setColumnCount(3);
    m_joints_table_view.setWindowTitle("Joints info for " + instance->getMetadata().getFileName());
    m_joints_table_view.setModel(&m_joints_model);
    m_joints_table_view.setMinimumSize(500, 600);

    QStringList list;
    list << "pos X" << "pos Y" << "pos Z";
    m_joints_model.setHorizontalHeaderLabels(list);

    for (int i=0; i<20; ++i) {
        QStandardItem *item = new QStandardItem;
        m_joints_model.setVerticalHeaderItem(i, item);
        for (int j=0; j<3; ++j) {
            QStandardItem *item = new QStandardItem;
            m_joints_model.setItem(i, j, item);
        }
    }

    // Setup Distances model
    m_distances_model.setRowCount(20);
    m_distances_model.setColumnCount(20);
    m_distances_table_view.setWindowTitle("Distances info for " + instance->getMetadata().getFileName());
    m_distances_table_view.setModel(&m_distances_model);
    m_distances_table_view.setMinimumSize(600, 600);

    const QMetaObject &mo = dai::SkeletonJoint::staticMetaObject;
    int index = mo.indexOfEnumerator("JointType"); // watch out during refactorings
    QMetaEnum metaEnum = mo.enumerator(index);

    list.clear();;

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
            QStandardItem *item = new QStandardItem;
            m_distances_model.setItem(i, j, item);
        }
    }

    // Show Tables
    m_joints_table_view.setWindowOpacity(0.85);
    m_joints_table_view.show();
    m_distances_table_view.setWindowOpacity(0.85);
    m_distances_table_view.show();
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


    while (it.hasNext())
    {
        const QMetaObject &mo = dai::SkeletonJoint::staticMetaObject;
        int index = mo.indexOfEnumerator("JointType"); // watch out during refactorings
        QMetaEnum metaEnum = mo.enumerator(index);
        dai::SkeletonJoint* joint = it.next();

        //QStandardItem* itemName = m_model.item(row, 0);
        QStandardItem* itemX = m_joints_model.item(row, 0);
        QStandardItem* itemY = m_joints_model.item(row, 1);
        QStandardItem* itemZ = m_joints_model.item(row, 2);

        QString name(metaEnum.valueToKey(joint->getType()));
        name = name.mid(6);

        m_joints_model.verticalHeaderItem(row)->setText(name);
        itemX->setText(QString::number(joint->getPosition().x()));
        itemY->setText(QString::number(joint->getPosition().y()));
        itemZ->setText(QString::number(joint->getPosition().z()));

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
            QStandardItem* item = m_distances_model.item(i, j);
            item->setText(QString::number(distance));
        }
    }

    m_distances_table_view.resizeColumnsToContents();
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
