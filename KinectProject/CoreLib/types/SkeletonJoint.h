#ifndef SKELETONJOINT_H
#define SKELETONJOINT_H

#include "Point3f.h"
#include <QObject>

namespace dai {

class SkeletonJoint : public QObject
{
    Q_OBJECT
    Q_ENUMS(JointType)

public:
    enum JointType {
        JOINT_HEAD,

        JOINT_CENTER_SHOULDER, // NECK on OpenNI
        JOINT_LEFT_SHOULDER,
        JOINT_RIGHT_SHOULDER,

        JOINT_LEFT_ELBOW,
        JOINT_RIGHT_ELBOW,

        JOINT_LEFT_WRIST,   // Kinect SDK (Not available in OpenNI)
        JOINT_RIGHT_WRIST, // Kinect SDK (Not available in OpenNI)

        JOINT_LEFT_HAND,
        JOINT_RIGHT_HAND,

        JOINT_SPINE, // TORSO on OpenNI

        JOINT_CENTER_HIP, // Kinect SDK (Not available in OpenNI)
        JOINT_LEFT_HIP,
        JOINT_RIGHT_HIP,

        JOINT_LEFT_KNEE,
        JOINT_RIGHT_KNEE,

        JOINT_LEFT_ANKLE, // Kinect SDK (Not available in OpenNI)
        JOINT_RIGHT_ANKLE, // Kinect SDK (Not available in OpenNI)

        JOINT_LEFT_FOOT,
        JOINT_RIGHT_FOOT
    };

    SkeletonJoint();
    explicit SkeletonJoint(const Point3f point, JointType type);
    SkeletonJoint(const SkeletonJoint& other);
    void setType(JointType type);
    void setPosition(const Point3f point);
    //void setScreenCoordinates(const Point3f point);
    const Point3f& getPosition() const;
    //const Point3f& getScreenPosition() const;
    SkeletonJoint& operator=(const SkeletonJoint& other);
    JointType getType() const;

private:
    Point3f     m_realworld_point;    // Real World Point in meters
    //Point3f     m_screenCoordinates;
    JointType   m_type;               // Joint Type
};

} // End Namespace

#endif // SKELETONJOINT_H
