#ifndef SKELETON_H
#define SKELETON_H

#include "DataFrame.h"
#include "SkeletonJoint.h"
#include <QHash>

namespace dai {

class Skeleton : public DataFrame
{
public:
    Skeleton();
    virtual ~Skeleton();
    const SkeletonJoint& getJoint(SkeletonJoint::JointType type) const;
    void addJoint(SkeletonJoint::JointType type, SkeletonJoint* joint);

private:
    QHash<SkeletonJoint::JointType, SkeletonJoint*> m_joints;
};

} // End Namespace

#endif // SKELETON_H
