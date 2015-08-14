#include "SkeletonJoint.h"

namespace dai {

SkeletonJoint::SkeletonJoint()
{
    m_type = JOINT_HEAD;
    m_position_confidence = 0.0f;
    m_orientation_confidence = 0.0f;
}

SkeletonJoint::SkeletonJoint(const Point3f& point, JointType type)
{
    m_realworld_point = point;
    m_type = type;
    m_position_confidence = 1.0f;
    m_orientation_confidence = 0.0f;
}

SkeletonJoint::SkeletonJoint(const SkeletonJoint& other)
{
    m_realworld_point = other.m_realworld_point;
    m_orientation = other.m_orientation;
    m_type = other.m_type;
    m_position_confidence = other.m_position_confidence;
    m_orientation_confidence = other.m_orientation_confidence;
}

SkeletonJoint& SkeletonJoint::operator=(const SkeletonJoint& other)
{
    m_realworld_point = other.m_realworld_point;
    m_orientation = other.m_orientation;
    m_type = other.m_type;
    m_position_confidence = other.m_position_confidence;
    m_orientation_confidence = other.m_orientation_confidence;
    return *this;
}

void SkeletonJoint::setType(JointType type)
{
    m_type = type;
}

void SkeletonJoint::setPosition(const Point3f &point)
{
    m_realworld_point = point;
}

void SkeletonJoint::setPositionConfidence(float value)
{
    m_position_confidence = value;
}

void SkeletonJoint::setOrientation(const Quaternion& orientation)
{
    m_orientation = orientation;
}

void SkeletonJoint::setOrientationConfidence(float value)
{
    m_orientation_confidence = value;
}

const Point3f& SkeletonJoint::getPosition() const
{
    return m_realworld_point;
}

float SkeletonJoint::getPositionConfidence() const
{
    return m_position_confidence;
}

const Quaternion& SkeletonJoint::getOrientation() const
{
    return m_orientation;
}

float SkeletonJoint::getOrientationConfidence() const
{
    return m_orientation_confidence;
}

SkeletonJoint::JointType SkeletonJoint::getType() const
{
    return m_type;
}

} // End Namespace
