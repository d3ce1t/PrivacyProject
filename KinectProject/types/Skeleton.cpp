#include "Skeleton.h"
#include <limits>
#include <QDebug>
#include <dataset/DataInstance.h>

namespace dai {

Skeleton::Skeleton()
{
    m_joints[SkeletonJoint::JOINT_HEAD].setType(SkeletonJoint::JOINT_HEAD);
    m_joints[SkeletonJoint::JOINT_CENTER_SHOULDER].setType(SkeletonJoint::JOINT_CENTER_SHOULDER);
    m_joints[SkeletonJoint::JOINT_LEFT_SHOULDER].setType(SkeletonJoint::JOINT_LEFT_SHOULDER);
    m_joints[SkeletonJoint::JOINT_RIGHT_SHOULDER].setType(SkeletonJoint::JOINT_RIGHT_SHOULDER);
    m_joints[SkeletonJoint::JOINT_LEFT_ELBOW].setType(SkeletonJoint::JOINT_LEFT_ELBOW);
    m_joints[SkeletonJoint::JOINT_RIGHT_ELBOW].setType(SkeletonJoint::JOINT_RIGHT_ELBOW);
    m_joints[SkeletonJoint::JOINT_LEFT_WRIST].setType(SkeletonJoint::JOINT_LEFT_WRIST);
    m_joints[SkeletonJoint::JOINT_RIGHT_WRIST].setType(SkeletonJoint::JOINT_RIGHT_WRIST);
    m_joints[SkeletonJoint::JOINT_LEFT_HAND].setType(SkeletonJoint::JOINT_LEFT_HAND);
    m_joints[SkeletonJoint::JOINT_RIGHT_HAND].setType(SkeletonJoint::JOINT_RIGHT_HAND);
    m_joints[SkeletonJoint::JOINT_SPINE].setType(SkeletonJoint::JOINT_SPINE);
    m_joints[SkeletonJoint::JOINT_CENTER_HIP].setType(SkeletonJoint::JOINT_CENTER_HIP);
    m_joints[SkeletonJoint::JOINT_LEFT_HIP].setType(SkeletonJoint::JOINT_LEFT_HIP);
    m_joints[SkeletonJoint::JOINT_RIGHT_HIP].setType(SkeletonJoint::JOINT_RIGHT_HIP);
    m_joints[SkeletonJoint::JOINT_LEFT_KNEE].setType(SkeletonJoint::JOINT_LEFT_KNEE);
    m_joints[SkeletonJoint::JOINT_RIGHT_KNEE].setType(SkeletonJoint::JOINT_RIGHT_KNEE);
    m_joints[SkeletonJoint::JOINT_LEFT_ANKLE].setType(SkeletonJoint::JOINT_LEFT_ANKLE);
    m_joints[SkeletonJoint::JOINT_RIGHT_ANKLE].setType(SkeletonJoint::JOINT_RIGHT_ANKLE);
    m_joints[SkeletonJoint::JOINT_LEFT_FOOT].setType(SkeletonJoint::JOINT_LEFT_FOOT);
    m_joints[SkeletonJoint::JOINT_RIGHT_FOOT].setType(SkeletonJoint::JOINT_RIGHT_FOOT);

    int map[17][2] = {
        {3, 1}, // Q1
        {2, 1}, // Q2
        {3, 5}, // Q3
        {2, 4}, // Q4
        {5, 7}, // Q5
        {4, 6}, // Q6
        {7, 9}, // Q7
        {6, 8}, // Q8
        {11, 10}, // Q9
        {13, 11}, // Q10
        {12, 11}, // Q11
        {13, 15}, // Q12
        {12, 14}, // Q13
        {15, 17}, // Q14
        {14, 16}, // Q15
        {17, 19}, // Q16
        {16, 18} // Q17
    };

    memcpy(m_map, map, 17*2*sizeof(int));

    for (int i=0; i<MAX_JOINTS; ++i) {
        m_normalised_joints[i].setType(m_joints[i].getType());
    }
}

Skeleton::Skeleton(const Skeleton& other)
{
    for (int i=0; i<MAX_JOINTS; ++i) {
        m_joints[i] = other.m_joints[i];
        m_normalised_joints[i] = other.m_normalised_joints[i];
    }

    m_joints_holder = other.m_joints_holder;
    m_normalised_joints_holder = other.m_normalised_joints_holder;

    for (int i=0; i<4; ++i)
        depth_norm_factors[i] = other.depth_norm_factors[i];

    depthNormEnabled = other.depthNormEnabled;

    for (int i=0; i<MAX_JOINTS-1; ++i) {
        m_vectors[i] = other.m_vectors[i];
    }

    for (int i=0; i<MAX_JOINTS-3; ++i) {
        m_quaternions[i] = other.m_quaternions[i];
    }
}

Skeleton::~Skeleton()
{
    clear();
}

Skeleton& Skeleton::operator=(const Skeleton& other)
{
    for (int i=0; i<MAX_JOINTS; ++i) {
        m_joints[i] = other.m_joints[i];
        m_normalised_joints[i] = other.m_normalised_joints[i];
    }

    m_joints_holder = other.m_joints_holder;
    m_normalised_joints_holder = other.m_normalised_joints_holder;

    for (int i=0; i<4; ++i)
        depth_norm_factors[i] = other.depth_norm_factors[i];

    depthNormEnabled = other.depthNormEnabled;

    for (int i=0; i<MAX_JOINTS-1; ++i) {
        m_vectors[i] = other.m_vectors[i];
    }

    for (int i=0; i<MAX_JOINTS-3; ++i) {
        m_quaternions[i] = other.m_quaternions[i];
    }

    return *this;
}

SkeletonJoint& Skeleton::getJoint(SkeletonJoint::JointType type)
{
    return m_joints[type];
}

SkeletonJoint &Skeleton::getNormalisedJoint(SkeletonJoint::JointType type)
{
    return m_normalised_joints[type];
}

const QList<SkeletonJoint*>& Skeleton::getAllJoints()
{
    m_joints_holder.clear();

    for (int i=0; i<MAX_JOINTS; ++i) {
        m_joints_holder.append(&m_joints[i]);
    }

    return m_joints_holder;
}

const Quaternion& Skeleton::getQuaternion(Quaternion::QuaternionType type) const
{
    return m_quaternions[type];
}

const SkeletonVector& Skeleton::getVector(SkeletonVector::VectorType type) const
{
    return m_vectors[type];
}

void Skeleton::normaliseDepth(float min, float max, float new_min, float new_max)
{
    depth_norm_factors[0] = min;
    depth_norm_factors[1] = max;
    depth_norm_factors[2] = new_min;
    depth_norm_factors[3] = new_max;
    depthNormEnabled = true;

    for (int i=0; i<MAX_JOINTS; ++i)
    {
        const SkeletonJoint* joint = &m_joints[i];
        float x = joint->getPosition().x();
        float y = joint->getPosition().y();
        float z = DataInstance::normalise(joint->getPosition().z(), depth_norm_factors[0], depth_norm_factors[1], depth_norm_factors[2], depth_norm_factors[3]);
        m_normalised_joints[i].setPosition( Point3f(x, y, z));
    }
}

void Skeleton::computeVectors()
{
    // Unitary Vectors computed by real world coordinates
    m_vectors[SkeletonVector::VECTOR_V1].setVector(&m_joints[SkeletonJoint::JOINT_CENTER_SHOULDER], &m_joints[SkeletonJoint::JOINT_HEAD]);
    m_vectors[SkeletonVector::VECTOR_V2].setVector(&m_joints[SkeletonJoint::JOINT_CENTER_SHOULDER], &m_joints[SkeletonJoint::JOINT_RIGHT_SHOULDER]);
    m_vectors[SkeletonVector::VECTOR_V3].setVector(&m_joints[SkeletonJoint::JOINT_CENTER_SHOULDER], &m_joints[SkeletonJoint::JOINT_LEFT_SHOULDER]);
    m_vectors[SkeletonVector::VECTOR_V4].setVector(&m_joints[SkeletonJoint::JOINT_RIGHT_SHOULDER], &m_joints[SkeletonJoint::JOINT_RIGHT_ELBOW]);
    m_vectors[SkeletonVector::VECTOR_V5].setVector(&m_joints[SkeletonJoint::JOINT_LEFT_SHOULDER], &m_joints[SkeletonJoint::JOINT_LEFT_ELBOW]);
    m_vectors[SkeletonVector::VECTOR_V6].setVector(&m_joints[SkeletonJoint::JOINT_RIGHT_ELBOW], &m_joints[SkeletonJoint::JOINT_RIGHT_WRIST]);
    m_vectors[SkeletonVector::VECTOR_V7].setVector(&m_joints[SkeletonJoint::JOINT_LEFT_ELBOW], &m_joints[SkeletonJoint::JOINT_LEFT_WRIST]);
    m_vectors[SkeletonVector::VECTOR_V8].setVector(&m_joints[SkeletonJoint::JOINT_RIGHT_WRIST], &m_joints[SkeletonJoint::JOINT_RIGHT_HAND]);
    m_vectors[SkeletonVector::VECTOR_V9].setVector(&m_joints[SkeletonJoint::JOINT_LEFT_WRIST], &m_joints[SkeletonJoint::JOINT_LEFT_HAND]);
    m_vectors[SkeletonVector::VECTOR_V10].setVector(&m_joints[SkeletonJoint::JOINT_SPINE], &m_joints[SkeletonJoint::JOINT_CENTER_SHOULDER]);
    m_vectors[SkeletonVector::VECTOR_V11].setVector(&m_joints[SkeletonJoint::JOINT_CENTER_HIP], &m_joints[SkeletonJoint::JOINT_SPINE]);
    m_vectors[SkeletonVector::VECTOR_V12].setVector(&m_joints[SkeletonJoint::JOINT_CENTER_HIP], &m_joints[SkeletonJoint::JOINT_RIGHT_HIP]);
    m_vectors[SkeletonVector::VECTOR_V13].setVector(&m_joints[SkeletonJoint::JOINT_CENTER_HIP], &m_joints[SkeletonJoint::JOINT_LEFT_HIP]);
    m_vectors[SkeletonVector::VECTOR_V14].setVector(&m_joints[SkeletonJoint::JOINT_RIGHT_HIP], &m_joints[SkeletonJoint::JOINT_RIGHT_KNEE]);
    m_vectors[SkeletonVector::VECTOR_V15].setVector(&m_joints[SkeletonJoint::JOINT_LEFT_HIP], &m_joints[SkeletonJoint::JOINT_LEFT_KNEE]);
    m_vectors[SkeletonVector::VECTOR_V16].setVector(&m_joints[SkeletonJoint::JOINT_RIGHT_KNEE], &m_joints[SkeletonJoint::JOINT_RIGHT_ANKLE]);
    m_vectors[SkeletonVector::VECTOR_V17].setVector(&m_joints[SkeletonJoint::JOINT_LEFT_KNEE], &m_joints[SkeletonJoint::JOINT_LEFT_ANKLE]);
    m_vectors[SkeletonVector::VECTOR_V18].setVector(&m_joints[SkeletonJoint::JOINT_RIGHT_ANKLE], &m_joints[SkeletonJoint::JOINT_RIGHT_FOOT]);
    m_vectors[SkeletonVector::VECTOR_V19].setVector(&m_joints[SkeletonJoint::JOINT_LEFT_ANKLE], &m_joints[SkeletonJoint::JOINT_LEFT_FOOT]);

    // Quaternions
    for (int i=0; i<17; ++i) {
        int offset_v1 = m_map[i][0]-1;
        int offset_v2 = m_map[i][1]-1;
        const QVector3D& v1 = m_vectors[offset_v1].vector();
        const QVector3D& v2 = m_vectors[offset_v2].vector();
        m_quaternions[i] = Quaternion::getRotationBetween(v1, v2);
    }

}

void Skeleton::mapQuaternionToVectors(Quaternion::QuaternionType type, SkeletonVector::VectorType* v1, SkeletonVector::VectorType* v2)
{
    *v1 = (SkeletonVector::VectorType) (m_map[type][0]-1);
    *v2 = (SkeletonVector::VectorType) (m_map[type][1]-1);
}

 void Skeleton::clear()
 {
     m_joints_holder.clear();
     m_normalised_joints_holder.clear();
 }

//
// Static methods
//
QVector3D Skeleton::maxValue(const Skeleton& frame)
{
    float bestXValue = std::numeric_limits<int>::min();
    float bestYValue = std::numeric_limits<int>::min();
    float bestZValue = std::numeric_limits<int>::min();

    for (int i=0; i<MAX_JOINTS; ++i)
    {
        const SkeletonJoint* joint = &(frame.m_joints[i]);

        float tempX = joint->getPosition().x();
        float tempY = joint->getPosition().y();
        float tempZ = joint->getPosition().z();

        if(tempX > bestXValue)
        {
            bestXValue = tempX;
        }

        if(tempY > bestYValue)
        {
            bestYValue = tempY;
        }

        if(tempZ > bestZValue)
        {
            bestZValue = tempZ;
        }
    }

    if (bestXValue >= 1)
        qDebug() << "MaxX: " << bestXValue;

    if (bestYValue >= 1)
        qDebug() << "MaxY: " << bestYValue;

    return QVector3D(bestXValue, bestYValue, bestZValue);
}

QVector3D Skeleton::minValue(const Skeleton &frame)
{
    float bestXValue = std::numeric_limits<int>::max();
    float bestYValue = std::numeric_limits<int>::max();
    float bestZValue = std::numeric_limits<int>::max();

    for (int i=0; i<MAX_JOINTS; ++i)
    {
        const SkeletonJoint* joint = &(frame.m_joints[i]);

        float tempX = joint->getPosition().x();
        float tempY = joint->getPosition().y();
        float tempZ = joint->getPosition().z();

        if(tempX < bestXValue)
        {
            bestXValue = tempX;
        }

        if(tempY < bestYValue)
        {
            bestYValue = tempY;
        }

        if(tempZ < bestZValue)
        {
            bestZValue = tempZ;
        }
    }

    if (bestXValue <= -1)
        qDebug() << "Min: " << bestXValue;

    if (bestYValue <= -1)
        qDebug() << "MinY: " << bestYValue;

    return QVector3D(bestXValue, bestYValue, bestZValue);
}

} // End Namespace
