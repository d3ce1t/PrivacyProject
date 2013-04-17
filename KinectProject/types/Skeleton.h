#ifndef SKELETON_H
#define SKELETON_H

#include "DataFrame.h"
#include "SkeletonJoint.h"
#include <QVector3D>
#include <QList>

namespace dai {

class Skeleton : public DataFrame
{
public:
    static QVector3D maxValue(const Skeleton &frame);
    static QVector3D minValue(const Skeleton& frame);

    Skeleton();
    Skeleton(const Skeleton& other);
    virtual ~Skeleton();
    SkeletonJoint& getNormalisedJoint(SkeletonJoint::JointType type);
    SkeletonJoint& getJoint(SkeletonJoint::JointType type);
    const QList<SkeletonJoint *> &getAllJoints();
    void normaliseDepth(float min, float max, float new_min, float new_max);
    void clear();

    // Overriden operators
    Skeleton& operator=(const Skeleton& other);

private:
    SkeletonJoint m_joints[20];
    QList<SkeletonJoint*> m_joints_holder;
    SkeletonJoint m_normalised_joints[20];
    QList<SkeletonJoint*> m_normalised_joints_holder;
    float depth_norm_factors[4]; // 0 -> min, 1 -> max, 2 -> newMin, 3 -> newMax
    bool depthNormEnabled;
};

} // End Namespace

#endif // SKELETON_H
