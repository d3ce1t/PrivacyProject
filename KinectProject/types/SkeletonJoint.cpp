#include "SkeletonJoint.h"

namespace dai {

SkeletonJoint::SkeletonJoint(const Point3f point)
{
    m_realworld_point = point;
}

SkeletonJoint::SkeletonJoint(const SkeletonJoint& other)
{
    m_realworld_point = other.m_realworld_point;
}

SkeletonJoint& SkeletonJoint::operator=(const SkeletonJoint& other)
{
    m_realworld_point = other.m_realworld_point;
    return *this;
}

void SkeletonJoint::setPosition(const Point3f point)
{
    m_realworld_point = point;
}

const Point3f& SkeletonJoint::getPosition() const
{
    return m_realworld_point;
}

} // End Namespace
