#include "SkeletonJoint.h"

namespace dai {

SkeletonJoint::SkeletonJoint()
{
    m_type = JOINT_HEAD;
}

SkeletonJoint::SkeletonJoint(const Point3f point, JointType type)
{
    m_realworld_point = point;
    m_type = type;
}

SkeletonJoint::SkeletonJoint(const SkeletonJoint& other)
    : QObject(0)
{
    m_realworld_point = other.m_realworld_point;
    m_type = other.m_type;
    //m_screenCoordinates = other.m_screenCoordinates;
}

void SkeletonJoint::setType(JointType type)
{
    m_type = type;
}

SkeletonJoint& SkeletonJoint::operator=(const SkeletonJoint& other)
{
    m_realworld_point = other.m_realworld_point;
    m_type = other.m_type;
    //m_screenCoordinates = other.m_screenCoordinates;
    return *this;
}

void SkeletonJoint::setPosition(const Point3f point)
{
    m_realworld_point = point;
}

/*void SkeletonJoint::setScreenCoordinates(const Point3f point)
{
    m_screenCoordinates = point;
}*/

const Point3f& SkeletonJoint::getPosition() const
{
    return m_realworld_point;
}

/*const Point3f& SkeletonJoint::getScreenPosition() const
{
    return m_screenCoordinates;
}*/

SkeletonJoint::JointType SkeletonJoint::getType() const
{
    return m_type;
}

} // End Namespace
