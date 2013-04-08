#include "Skeleton.h"

namespace dai {

Skeleton::Skeleton()
{
}

Skeleton::~Skeleton()
{
}


const SkeletonJoint& Skeleton::getJoint(SkeletonJoint::JointType type) const
{
    return *(m_joints.value(type));
}

void Skeleton::addJoint(SkeletonJoint::JointType type, SkeletonJoint* joint)
{
    m_joints.insert(type, joint);
}

} // End Namespace
