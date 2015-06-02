#include "Skeleton.h"
#include <cmath>
#include <QVector>
#include <QDebug>

namespace dai {

// Quaternions Map
SkeletonJoint::JointType Skeleton::staticQuaternionsMap[22][3] = {
    {SkeletonJoint::JOINT_HEAD,             SkeletonJoint::JOINT_CENTER_SHOULDER,   SkeletonJoint::JOINT_SPINE}, // Q1
    {SkeletonJoint::JOINT_HEAD,             SkeletonJoint::JOINT_CENTER_SHOULDER,   SkeletonJoint::JOINT_LEFT_SHOULDER}, // Q2
    {SkeletonJoint::JOINT_HEAD,             SkeletonJoint::JOINT_CENTER_SHOULDER,   SkeletonJoint::JOINT_RIGHT_SHOULDER}, // Q3
    {SkeletonJoint::JOINT_CENTER_SHOULDER,  SkeletonJoint::JOINT_LEFT_SHOULDER,     SkeletonJoint::JOINT_LEFT_ELBOW}, // Q4
    {SkeletonJoint::JOINT_CENTER_SHOULDER,  SkeletonJoint::JOINT_RIGHT_SHOULDER,    SkeletonJoint::JOINT_RIGHT_ELBOW}, // Q5
    {SkeletonJoint::JOINT_LEFT_SHOULDER,    SkeletonJoint::JOINT_LEFT_ELBOW,        SkeletonJoint::JOINT_LEFT_WRIST}, // Q6
    {SkeletonJoint::JOINT_RIGHT_SHOULDER,   SkeletonJoint::JOINT_RIGHT_ELBOW,       SkeletonJoint::JOINT_RIGHT_WRIST}, // Q7
    {SkeletonJoint::JOINT_LEFT_ELBOW,       SkeletonJoint::JOINT_LEFT_WRIST,        SkeletonJoint::JOINT_LEFT_HAND}, // Q8
    {SkeletonJoint::JOINT_RIGHT_ELBOW,      SkeletonJoint::JOINT_RIGHT_WRIST,       SkeletonJoint::JOINT_RIGHT_HAND}, // Q9
    {SkeletonJoint::JOINT_LEFT_SHOULDER,    SkeletonJoint::JOINT_CENTER_SHOULDER,   SkeletonJoint::JOINT_RIGHT_SHOULDER}, // Q10
    {SkeletonJoint::JOINT_CENTER_SHOULDER,  SkeletonJoint::JOINT_SPINE,             SkeletonJoint::JOINT_CENTER_HIP}, // Q11
    {SkeletonJoint::JOINT_SPINE,            SkeletonJoint::JOINT_CENTER_HIP,        SkeletonJoint::JOINT_LEFT_HIP}, // Q12
    {SkeletonJoint::JOINT_SPINE,            SkeletonJoint::JOINT_CENTER_HIP,        SkeletonJoint::JOINT_RIGHT_HIP}, // Q13
    {SkeletonJoint::JOINT_CENTER_HIP,       SkeletonJoint::JOINT_LEFT_HIP,          SkeletonJoint::JOINT_LEFT_KNEE}, // Q14
    {SkeletonJoint::JOINT_CENTER_HIP,       SkeletonJoint::JOINT_RIGHT_HIP,         SkeletonJoint::JOINT_RIGHT_KNEE}, // Q15
    {SkeletonJoint::JOINT_LEFT_HIP,         SkeletonJoint::JOINT_LEFT_KNEE,         SkeletonJoint::JOINT_LEFT_ANKLE}, // Q16
    {SkeletonJoint::JOINT_RIGHT_HIP,        SkeletonJoint::JOINT_RIGHT_KNEE,        SkeletonJoint::JOINT_RIGHT_ANKLE}, // Q17
    {SkeletonJoint::JOINT_LEFT_KNEE,        SkeletonJoint::JOINT_LEFT_ANKLE,        SkeletonJoint::JOINT_LEFT_FOOT}, // Q18
    {SkeletonJoint::JOINT_RIGHT_KNEE,       SkeletonJoint::JOINT_RIGHT_ANKLE,       SkeletonJoint::JOINT_RIGHT_FOOT}, // Q19
    {SkeletonJoint::JOINT_LEFT_HIP,         SkeletonJoint::JOINT_CENTER_HIP,        SkeletonJoint::JOINT_RIGHT_HIP}, // Q20
    {SkeletonJoint::JOINT_LEFT_SHOULDER,    SkeletonJoint::JOINT_CENTER_SHOULDER,   SkeletonJoint::JOINT_SPINE}, // Q21
    {SkeletonJoint::JOINT_RIGHT_SHOULDER,   SkeletonJoint::JOINT_CENTER_SHOULDER,   SkeletonJoint::JOINT_SPINE}, // Q22
};

// Kinect Limbs
Skeleton::SkeletonLimb Skeleton::staticKinectLimbsMap[MAX_LIMBS] = {
    // Center
    {SkeletonJoint::JOINT_HEAD, SkeletonJoint::JOINT_CENTER_SHOULDER},
    {SkeletonJoint::JOINT_CENTER_SHOULDER, SkeletonJoint::JOINT_SPINE},
    {SkeletonJoint::JOINT_SPINE, SkeletonJoint::JOINT_CENTER_HIP},
    // Left
    {SkeletonJoint::JOINT_CENTER_SHOULDER, SkeletonJoint::JOINT_LEFT_SHOULDER},
    {SkeletonJoint::JOINT_LEFT_SHOULDER, SkeletonJoint::JOINT_LEFT_ELBOW},
    {SkeletonJoint::JOINT_LEFT_ELBOW, SkeletonJoint::JOINT_LEFT_WRIST},
    {SkeletonJoint::JOINT_LEFT_WRIST, SkeletonJoint::JOINT_LEFT_HAND},
    {SkeletonJoint::JOINT_CENTER_HIP, SkeletonJoint::JOINT_LEFT_HIP},
    {SkeletonJoint::JOINT_LEFT_HIP, SkeletonJoint::JOINT_LEFT_KNEE},
    {SkeletonJoint::JOINT_LEFT_KNEE, SkeletonJoint::JOINT_LEFT_ANKLE},
    {SkeletonJoint::JOINT_LEFT_ANKLE, SkeletonJoint::JOINT_LEFT_FOOT},
    // Right
    {SkeletonJoint::JOINT_CENTER_SHOULDER, SkeletonJoint::JOINT_RIGHT_SHOULDER},
    {SkeletonJoint::JOINT_RIGHT_SHOULDER, SkeletonJoint::JOINT_RIGHT_ELBOW},
    {SkeletonJoint::JOINT_RIGHT_ELBOW, SkeletonJoint::JOINT_RIGHT_WRIST},
    {SkeletonJoint::JOINT_RIGHT_WRIST, SkeletonJoint::JOINT_RIGHT_HAND},
    {SkeletonJoint::JOINT_CENTER_HIP, SkeletonJoint::JOINT_RIGHT_HIP},
    {SkeletonJoint::JOINT_RIGHT_HIP, SkeletonJoint::JOINT_RIGHT_KNEE},
    {SkeletonJoint::JOINT_RIGHT_KNEE, SkeletonJoint::JOINT_RIGHT_ANKLE},
    {SkeletonJoint::JOINT_RIGHT_ANKLE, SkeletonJoint::JOINT_RIGHT_FOOT}
};

// OpenNI Limbs
Skeleton::SkeletonLimb Skeleton::staticOpenNILimbsMap[16] = {
    // Center
    {SkeletonJoint::JOINT_HEAD, SkeletonJoint::JOINT_CENTER_SHOULDER},
    {SkeletonJoint::JOINT_LEFT_SHOULDER, SkeletonJoint::JOINT_SPINE},
    {SkeletonJoint::JOINT_RIGHT_SHOULDER, SkeletonJoint::JOINT_SPINE},
    {SkeletonJoint::JOINT_SPINE, SkeletonJoint::JOINT_LEFT_HIP},
    {SkeletonJoint::JOINT_SPINE, SkeletonJoint::JOINT_RIGHT_HIP},
    {SkeletonJoint::JOINT_LEFT_HIP, SkeletonJoint::JOINT_RIGHT_HIP},
    // Left
    {SkeletonJoint::JOINT_CENTER_SHOULDER, SkeletonJoint::JOINT_LEFT_SHOULDER},
    {SkeletonJoint::JOINT_LEFT_SHOULDER, SkeletonJoint::JOINT_LEFT_ELBOW},
    {SkeletonJoint::JOINT_LEFT_ELBOW, SkeletonJoint::JOINT_LEFT_HAND},
    {SkeletonJoint::JOINT_LEFT_HIP, SkeletonJoint::JOINT_LEFT_KNEE},
    {SkeletonJoint::JOINT_LEFT_KNEE, SkeletonJoint::JOINT_LEFT_FOOT},
    // Right
    {SkeletonJoint::JOINT_CENTER_SHOULDER, SkeletonJoint::JOINT_RIGHT_SHOULDER},
    {SkeletonJoint::JOINT_RIGHT_SHOULDER, SkeletonJoint::JOINT_RIGHT_ELBOW},
    {SkeletonJoint::JOINT_RIGHT_ELBOW, SkeletonJoint::JOINT_RIGHT_HAND},
    {SkeletonJoint::JOINT_RIGHT_HIP, SkeletonJoint::JOINT_RIGHT_KNEE},
    {SkeletonJoint::JOINT_RIGHT_KNEE, SkeletonJoint::JOINT_RIGHT_FOOT}
};

Skeleton::Skeleton(SkeletonType type)
{
    m_type = type;
    m_units = dai::DISTANCE_MILIMETERS;

    if (type == SKELETON_OPENNI) {
        memcpy(m_limbs, staticOpenNILimbsMap, 16 * sizeof(SkeletonLimb));
        m_limbsSize = 16;
    } else {
        memcpy(m_limbs, staticKinectLimbsMap, 19 * sizeof(SkeletonLimb));
        m_limbsSize = 19;
    }
}

Skeleton::Skeleton(const Skeleton& other)
{
    m_type = other.m_type;
    m_limbsSize = other.m_limbsSize;
    m_joints = other.m_joints;  // implicit sharing
    m_quaternions = other.m_quaternions; // implicit sharing
    m_units = other.m_units;
    memcpy(m_limbs, other.m_limbs, other.m_limbsSize * sizeof(SkeletonLimb));
}

Skeleton& Skeleton::operator=(const Skeleton& other)
{
    m_type = other.m_type;
    m_limbsSize = other.m_limbsSize;
    m_joints = other.m_joints;
    m_quaternions = other.m_quaternions;
    m_units = other.m_units;
    memcpy(m_limbs, other.m_limbs, other.m_limbsSize * sizeof(SkeletonLimb));
    return *this;
}

const SkeletonJoint Skeleton::getJoint(SkeletonJoint::JointType type) const
{
    return m_joints.value(type);
}

QList<SkeletonJoint> Skeleton::joints() const
{
    return m_joints.values();
}

const Quaternion Skeleton::getQuaternion(Quaternion::QuaternionType type) const
{
    return m_quaternions.value(type);
}

QList<Quaternion> Skeleton::quaternions() const
{
    return m_quaternions.values();
}

const Skeleton::SkeletonLimb* Skeleton::getLimbsMap() const
{
    return m_limbs;
}

short Skeleton::getJointsCount() const
{
    return m_joints.size();
}

short Skeleton::getLimbsCount() const
{
    return m_limbsSize;
}

Skeleton::SkeletonType Skeleton::getType() const
{
    return m_type;
}

DistanceUnits Skeleton::distanceUnits() const
{
    return m_units;
}

void Skeleton::setJoint(SkeletonJoint::JointType type, const SkeletonJoint& joint)
{
    m_joints.insert(type, joint); // copy
}

void Skeleton::setDistanceUnits(DistanceUnits units)
{
    m_units = units;
}

void Skeleton::computeQuaternions()
{
    // Quaternions
    for (int i=0; i<22; ++i) {
        SkeletonJoint::JointType joint1 = staticQuaternionsMap[i][0];
        SkeletonJoint::JointType joint2 = staticQuaternionsMap[i][1]; // vertex
        SkeletonJoint::JointType joint3 = staticQuaternionsMap[i][2];
        m_quaternions[i] = Quaternion::getRotationBetween(m_joints[joint1].getPosition(),
                                                          m_joints[joint3].getPosition(),
                                                          m_joints[joint2].getPosition());
    }
}

// http://ksimek.github.io/2013/08/13/intrinsic/
void Skeleton::convertJointCoordinatesToDepth(float x, float y, float z, float* pOutX, float* pOutY)
{
    const double fd_x = 594.21434211923247;
    const double fd_y = 591.04053696870778;
    const double cd_x = 640 / 2.0;
    const double cd_y = 480 / 2.0;
    *pOutX = x * fd_x / z + cd_x;
    *pOutY = 480 - (y * fd_y / z + cd_y);
}

QByteArray Skeleton::toBinary() const
{
    QByteArray data_mem(m_joints.size() * 37 + 2, 0);
    uchar* pData = (uchar*) data_mem.data();

    *pData++ = m_joints.size(); // Number of joints (1 byte)
    *pData++ = m_units; // Units (1 byte)

    for (SkeletonJoint joint : m_joints)
    {
        *pData = joint.getType(); // Joint type (1 byte)
        pData++;

        float* pItem = (float*) pData;

        *pItem = joint.getPosition()[0]; // pos.x (4 bytes)
        pItem++;

        *pItem = joint.getPosition()[1]; // pos.y (4 bytes)
        pItem++;

        *pItem = joint.getPosition()[2]; // pos.z (4 bytes)
        pItem++;

        *pItem = joint.getOrientation().w(); // w (4 bytes)
        pItem++;

        *pItem = joint.getOrientation().x(); // x (4 bytes)
        pItem++;

        *pItem = joint.getOrientation().y(); // y (4 bytes)
        pItem++;

        *pItem = joint.getOrientation().z(); // z (4 bytes)
        pItem++;

        *pItem = joint.getPositionConfidence(); // pos confidence (4 bytes)
        pItem++;

        *pItem = joint.getOrientationConfidence(); // orientation confidence (4 bytes)
        pItem++;

        // Move pointer
        pData = (uchar*) pItem;
    }

    return data_mem;
}

SkeletonPtr Skeleton::fromBinary(const QByteArray& buffer, int* read_bytes)
{
    int read_bytes_tmp = 0;
    uchar* binData = (uchar*) buffer.data();
    uchar number_joints = *binData++;
    DistanceUnits units = (DistanceUnits) *binData++;

    SkeletonType skelType;

    if (number_joints == 15)
        skelType = SKELETON_OPENNI;
    else if (number_joints == 20)
        skelType = SKELETON_KINECT;

    std::shared_ptr<Skeleton> skeleton = std::make_shared<Skeleton>(skelType);
    skeleton->setDistanceUnits(units);

    read_bytes_tmp = 2;

    for (int i=0; i<number_joints; ++i)
    {
        SkeletonJoint::JointType type = (SkeletonJoint::JointType) *binData;
        binData++;

        float* pItem = (float*) binData;
        Point3f point;
        point[0] = *pItem++;
        point[1] = *pItem++;
        point[2] = *pItem++;
        float q_w = *pItem++;
        float q_x = *pItem++;
        float q_y = *pItem++;
        float q_z = *pItem++;
        float pos_conf = *pItem++;
        float or_conf = *pItem++;

        SkeletonJoint joint(point, type);
        Quaternion quaternion(q_w, q_x, q_y, q_z);
        joint.setPositionConfidence(pos_conf);
        joint.setOrientationConfidence(or_conf);
        joint.setOrientation(quaternion);

        // Add to skeleton
        skeleton->setJoint(type, joint);
        SkeletonJoint other = skeleton->getJoint(type);

        // Move pointer
        binData = (uchar*) pItem;
        read_bytes_tmp += 37;
    }

    if (read_bytes)
        *read_bytes = read_bytes_tmp;

    return skeleton;
}

} // End Namespace
