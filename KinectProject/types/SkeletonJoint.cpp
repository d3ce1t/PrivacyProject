#include "SkeletonJoint.h"

namespace dai {

SkeletonJoint::SkeletonJoint()
{
}

SkeletonJoint::SkeletonJoint(const Point3f point)
{
    m_point = point;
}

SkeletonJoint::SkeletonJoint(const Point3f point, const Quaternion orientation)
{
    m_point = point;
    m_orientation = orientation;
}

void SkeletonJoint::setScreenPosition(const Point3f point)
{
    m_screenPoint = point;
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
    return m_point;
}

const Quaternion& SkeletonJoint::getOrientation() const
{
    return m_orientation;
}

const Point3f& SkeletonJoint::getScreenPosition() const
{
    return m_screenPoint;
}

} // End Namespace
