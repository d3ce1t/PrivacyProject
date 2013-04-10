#ifndef SKELETON_H
#define SKELETON_H

#include "DataFrame.h"
#include "SkeletonJoint.h"
#include <QHash>
#include <QVector3D>

namespace dai {

class Skeleton : public DataFrame
{
public:
    static QVector3D maxValue(const Skeleton &frame);
    static QVector3D minValue(const Skeleton& frame);

    Skeleton();
    virtual ~Skeleton();
    const SkeletonJoint& getJoint(SkeletonJoint::JointType type) const;
    void addJoint(SkeletonJoint::JointType type, SkeletonJoint* joint);

private:
    QHash<SkeletonJoint::JointType, SkeletonJoint*> m_joints;
};

} // End Namespace

#endif // SKELETON_H
