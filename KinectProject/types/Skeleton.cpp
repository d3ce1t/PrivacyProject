#include "Skeleton.h"

namespace dai {

// Quaternions Map
SkeletonJoint::JointType Skeleton::staticMap[20][3] = {
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

Skeleton::Skeleton(const Skeleton& other)
{
    for (int i=0; i<MAX_JOINTS; ++i) {
        m_joints[i] = other.m_joints[i];
        m_quaternions[i] = other.m_quaternions[i];
    }
}

Skeleton& Skeleton::operator=(const Skeleton& other)
{
    for (int i=0; i<MAX_JOINTS; ++i) {
        m_joints[i] = other.m_joints[i];
        m_quaternions[i] = other.m_quaternions[i];
    }

    return *this;
}

const SkeletonJoint &Skeleton::getJoint(SkeletonJoint::JointType type) const
{
    return m_joints[type];
}

const Quaternion& Skeleton::getQuaternion(Quaternion::QuaternionType type) const
{
    return m_quaternions[type];
}

int Skeleton::getJointsCount() const
{
    return MAX_JOINTS;
}

void Skeleton::setJoint(SkeletonJoint::JointType type, const SkeletonJoint& joint)
{
    m_joints[type] = joint; // copy
}

void Skeleton::computeQuaternions()
{
    // Quaternions
    for (int i=0; i<20; ++i) {
        SkeletonJoint::JointType joint1 = staticMap[i][0];
        SkeletonJoint::JointType joint2 = staticMap[i][1]; // vertex
        SkeletonJoint::JointType joint3 = staticMap[i][2];
        m_quaternions[i] = Quaternion::getRotationBetween(m_joints[joint1].getPosition(),
                                                          m_joints[joint3].getPosition(),
                                                          m_joints[joint2].getPosition());
    }
}

} // End Namespace
