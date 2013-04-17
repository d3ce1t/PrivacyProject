#include "SkeletonJoint.h"

namespace dai {

SkeletonJoint::SkeletonJoint()
{
}

SkeletonJoint::SkeletonJoint(const Point3f point)
{
    m_realworld_point = point;
}

SkeletonJoint::SkeletonJoint(const Point3f point, const Quaternion orientation)
{
    m_realworld_point = point;
    m_orientation = orientation;
}

SkeletonJoint::SkeletonJoint(const SkeletonJoint& other)
{
    m_type = other.m_type;
    m_realworld_point = other.m_realworld_point;
    m_screen_point = other.m_screen_point;
    m_orientation = other.m_orientation;
}

SkeletonJoint& SkeletonJoint::operator=(const SkeletonJoint& other)
{
    m_type = other.m_type;
    m_realworld_point = other.m_realworld_point;
    m_screen_point = other.m_screen_point;
    m_orientation = other.m_orientation;
}

void SkeletonJoint::setPosition(const Point3f point)
{
    m_realworld_point = point;
}

void SkeletonJoint::setScreenPosition(const Point3f point)
{
    m_screen_point = point;
}

void SkeletonJoint::setType(JointType type)
{
    m_type = type;
}

SkeletonJoint::JointType SkeletonJoint::getType() const
{
    return m_type;
}

const Point3f& SkeletonJoint::getPosition() const
{
    return m_realworld_point;
}

const Quaternion& SkeletonJoint::getOrientation() const
{
    return m_orientation;
}

const Point3f& SkeletonJoint::getScreenPosition() const
{
    return m_screen_point;
}

} // End Namespace
