#ifndef SKELETON_H
#define SKELETON_H

#include "SkeletonJoint.h"
#include "Quaternion.h"
#include "types/Enums.h"
#include <QVector3D>

#define MAX_JOINTS 20
#define MAX_LIMBS 19

namespace dai {

/**
 * Skeleton depth distances are in meter in real world coordinates.
 *
 * @brief The Skeleton class
 */
class Skeleton
{
    SkeletonJoint m_joints[MAX_JOINTS]; // joints with real world coordinates in meters
    Quaternion m_quaternions[MAX_JOINTS];
    SkeletonLimb m_limbs[MAX_LIMBS];
    short m_jointsCount;
    short m_limbsSize;
    SkeletonType m_type;
    SkeletonState m_state;
    DistanceUnits m_units;

public:
    enum SkeletonType {
        SKELETON_KINECT,
        SKELETON_OPENNI
    };

    enum SkeletonState {
        NEW_USER,
        LOST_USER
    };

    struct SkeletonLimb {
        SkeletonJoint::JointType joint1;
        SkeletonJoint::JointType joint2;
    };

    static void convertJointCoordinatesToDepth(float x, float y, float z, float* pOutX, float* pOutY);

    // Constructors
    Skeleton(SkeletonType type = SKELETON_OPENNI);
    Skeleton(const Skeleton& other);

    // Methods
    const SkeletonJoint& getJoint(SkeletonJoint::JointType type) const;
    const Quaternion& getQuaternion(Quaternion::QuaternionType type) const;
    const SkeletonLimb* getLimbsMap() const;
    short getJointsCount() const;
    short getLimbsCount() const;
    SkeletonType getType() const;
    DistanceUnits distanceUnits() const;
    void setJoint(SkeletonJoint::JointType type, const SkeletonJoint& joint);
    void computeQuaternions();
    void setDistanceUnits(DistanceUnits units);

    // Operators Overload
    Skeleton& operator=(const Skeleton& other);

private:
    static SkeletonJoint::JointType staticQuaternionsMap[20][3];
    static SkeletonLimb staticKinectLimbsMap[MAX_LIMBS];
    static SkeletonLimb staticOpenNILimbsMap[16];


};

} // End Namespace

#endif // SKELETON_H
