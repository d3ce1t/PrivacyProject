#include "Skeleton.h"
#include <limits>
#include <QDebug>
#include "../dataset/DataInstance.h"

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


    SkeletonJoint::JointType map[20][3] = {
        {SkeletonJoint::JOINT_HEAD,             SkeletonJoint::JOINT_CENTER_SHOULDER,   SkeletonJoint::JOINT_SPINE}, // Q1
        {SkeletonJoint::JOINT_HEAD,             SkeletonJoint::JOINT_CENTER_SHOULDER,   SkeletonJoint::JOINT_LEFT_SHOULDER}, // Q2
        {SkeletonJoint::JOINT_HEAD,             SkeletonJoint::JOINT_CENTER_SHOULDER,   SkeletonJoint::JOINT_RIGHT_SHOULDER}, // Q3
        {SkeletonJoint::JOINT_CENTER_SHOULDER,  SkeletonJoint::JOINT_LEFT_SHOULDER,     SkeletonJoint::JOINT_LEFT_ELBOW}, // Q4
        {SkeletonJoint::JOINT_CENTER_SHOULDER,  SkeletonJoint::JOINT_RIGHT_SHOULDER,    SkeletonJoint::JOINT_RIGHT_ELBOW}, // Q5
        {SkeletonJoint::JOINT_LEFT_SHOULDER,    SkeletonJoint::JOINT_LEFT_ELBOW,        SkeletonJoint::JOINT_LEFT_WRIST}, // Q6
        {SkeletonJoint::JOINT_RIGHT_SHOULDER,   SkeletonJoint::JOINT_RIGHT_ELBOW,       SkeletonJoint::JOINT_RIGHT_WRIST}, // Q7
        {SkeletonJoint::JOINT_LEFT_ELBOW,       SkeletonJoint::JOINT_LEFT_WRIST,        SkeletonJoint::JOINT_LEFT_HAND}, // Q8
        {SkeletonJoint::JOINT_RIGHT_ELBOW,      SkeletonJoint::JOINT_RIGHT_WRIST,       SkeletonJoint::JOINT_RIGHT_HAND}, // Q9
        {SkeletonJoint::JOINT_LEFT_SHOULDER,    SkeletonJoint::JOINT_CENTER_SHOULDER,   SkeletonJoint::JOINT_RIGHT_SHOULDER}, // Q10
        {SkeletonJoint::JOINT_CENTER_SHOULDER,  SkeletonJoint::JOINT_SPINE,             SkeletonJoint::JOINT_CENTER_HIP}, // Q11
        {SkeletonJoint::JOINT_SPINE,            SkeletonJoint::JOINT_CENTER_HIP,        SkeletonJoint::JOINT_LEFT_HIP}, // Q12
        {SkeletonJoint::JOINT_SPINE,            SkeletonJoint::JOINT_CENTER_HIP,        SkeletonJoint::JOINT_RIGHT_HIP}, // Q13
        {SkeletonJoint::JOINT_CENTER_HIP,       SkeletonJoint::JOINT_LEFT_HIP,          SkeletonJoint::JOINT_LEFT_KNEE}, // Q14
        {SkeletonJoint::JOINT_CENTER_HIP,       SkeletonJoint::JOINT_RIGHT_HIP,         SkeletonJoint::JOINT_RIGHT_KNEE}, // Q15
        {SkeletonJoint::JOINT_LEFT_HIP,         SkeletonJoint::JOINT_LEFT_KNEE,         SkeletonJoint::JOINT_LEFT_ANKLE}, // Q16
        {SkeletonJoint::JOINT_RIGHT_HIP,        SkeletonJoint::JOINT_RIGHT_KNEE,        SkeletonJoint::JOINT_RIGHT_ANKLE}, // Q17
        {SkeletonJoint::JOINT_LEFT_KNEE,        SkeletonJoint::JOINT_LEFT_ANKLE,        SkeletonJoint::JOINT_LEFT_FOOT}, // Q18
        {SkeletonJoint::JOINT_RIGHT_KNEE,       SkeletonJoint::JOINT_RIGHT_ANKLE,       SkeletonJoint::JOINT_RIGHT_FOOT}, // Q19
        {SkeletonJoint::JOINT_LEFT_HIP,         SkeletonJoint::JOINT_CENTER_HIP,        SkeletonJoint::JOINT_RIGHT_HIP} // Q20
    };

    memcpy(m_map, map, 20*3*sizeof(SkeletonJoint::JointType));

    for (int i=0; i<MAX_JOINTS; ++i) {
        m_normalised_joints[i].setType(m_joints[i].getType());
    }
}

Skeleton::Skeleton(const Skeleton& other)
    : DataFrame(other)
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

    for (int i=0; i<MAX_JOINTS; ++i) {
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

    for (int i=0; i<MAX_JOINTS; ++i) {
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

void Skeleton::computeQuaternions()
{
    // Quaternions
    for (int i=0; i<20; ++i) {
        SkeletonJoint::JointType joint1 = m_map[i][0];
        SkeletonJoint::JointType joint2 = m_map[i][1]; // vertex
        SkeletonJoint::JointType joint3 = m_map[i][2];
        m_quaternions[i] = Quaternion::getRotationBetween(m_joints[joint1].getPosition(),
                                                          m_joints[joint3].getPosition(),
                                                          m_joints[joint2].getPosition());
    }

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
    float bestXValue = std::numeric_limits<float>::min();
    float bestYValue = std::numeric_limits<float>::min();
    float bestZValue = std::numeric_limits<float>::min();

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
    float bestXValue = std::numeric_limits<float>::max();
    float bestYValue = std::numeric_limits<float>::max();
    float bestZValue = std::numeric_limits<float>::max();

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
