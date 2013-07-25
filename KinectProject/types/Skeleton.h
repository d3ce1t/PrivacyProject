#ifndef SKELETON_H
#define SKELETON_H

#include "DataFrame.h"
#include "SkeletonJoint.h"
#include "Quaternion.h"
#include <QVector3D>

#define MAX_JOINTS 20

namespace dai {

/**
 * Skeleton depth distances are in meter in real world coordinates.
 *
 * @brief The Skeleton class
 */
class Skeleton : public DataFrame
{
public:
    // Constructors
    Skeleton();
    Skeleton(const Skeleton& other);

    // Methods
    const SkeletonJoint& getJoint(SkeletonJoint::JointType type) const;
    const Quaternion& getQuaternion(Quaternion::QuaternionType type) const;
    int getJointsCount() const;
    shared_ptr<DataFrame> clone() const;
    void setJoint(SkeletonJoint::JointType type, const SkeletonJoint& joint);
    void computeQuaternions();

    // Operators Overload
    Skeleton& operator=(const Skeleton& other);

private:
    static SkeletonJoint::JointType m_map[20][3];

    SkeletonJoint m_joints[MAX_JOINTS]; // joints with real world coordinates in meters
    Quaternion m_quaternions[MAX_JOINTS];
};

} // End Namespace

#endif // SKELETON_H
