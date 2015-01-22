#ifndef SKELETON_H
#define SKELETON_H

#include "SkeletonJoint.h"
#include "Quaternion.h"
#include "types/Enums.h"
#include <QHash>
#include <QMap>
#include <memory>

#define MAX_LIMBS 19

namespace dai {

class Skeleton;
typedef std::shared_ptr<Skeleton> SkeletonPtr;

/**
 * Skeleton depth distances are in meter in real world coordinates.
 *
 * @brief The Skeleton class
 */
class Skeleton
{
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

    static SkeletonPtr fromBinary(const QByteArray &binData, int* read_bytes = nullptr);
    static void convertJointCoordinatesToDepth(float x, float y, float z, float* pOutX, float* pOutY);
    static void convertDepthCoordinatesToJoint(float x, float y, float z, float* pOutX, float* pOutY);

    // Constructors
    Skeleton(SkeletonType type = SKELETON_OPENNI);
    Skeleton(const Skeleton& other);

    // Methods
    const SkeletonJoint getJoint(SkeletonJoint::JointType type) const;
    QList<SkeletonJoint> joints() const;
    const Quaternion getQuaternion(Quaternion::QuaternionType type) const;
    QList<Quaternion> quaternions() const;
    const SkeletonLimb* getLimbsMap() const;
    short getJointsCount() const;
    short getLimbsCount() const;
    SkeletonType getType() const;
    DistanceUnits distanceUnits() const;
    void setJoint(SkeletonJoint::JointType type, const SkeletonJoint& joint);
    void computeQuaternions();
    void setDistanceUnits(DistanceUnits units);
    QByteArray toBinary() const;

    // Operators Overload
    Skeleton& operator=(const Skeleton& other);

private:
    static SkeletonJoint::JointType staticQuaternionsMap[22][3];
    static SkeletonLimb staticKinectLimbsMap[MAX_LIMBS];
    static SkeletonLimb staticOpenNILimbsMap[16];

    QMap<int, SkeletonJoint> m_joints;
    QMap<int, Quaternion> m_quaternions;
    SkeletonLimb m_limbs[MAX_LIMBS];
    short m_limbsSize;
    SkeletonType m_type;
    //SkeletonState m_state;
    DistanceUnits m_units;
};

} // End Namespace

#endif // SKELETON_H
