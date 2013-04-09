#include "Skeleton.h"
#include <limits>

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

float Skeleton::maxValue(const Skeleton& frame)
{
    QHashIterator<SkeletonJoint::JointType, SkeletonJoint*> it(frame.m_joints);

    float bestValue = std::numeric_limits<int>::min();

    while (it.hasNext())
    {
        it.next();
        SkeletonJoint* joint = it.value();

        float temp = joint->getPosition().z();

        if(temp > bestValue)
        {
            bestValue = temp;
        }
    }

    return bestValue;
}

float Skeleton::minValue(const Skeleton &frame)
{
    QHashIterator<SkeletonJoint::JointType, SkeletonJoint*> it(frame.m_joints);

    float bestValue = std::numeric_limits<int>::max();

    while (it.hasNext())
    {
        it.next();
        SkeletonJoint* joint = it.value();

        float temp = joint->getPosition().z();

        if(temp < bestValue)
        {
            bestValue = temp;
        }
    }

    return bestValue;
}

} // End Namespace
