#include "skeleton.h"
#include <QDebug>

using namespace dai;

Skeleton::Skeleton(QObject *parent) :
    QObject(parent)
{
    m_skeleton = NULL;
    m_pUserTracker = NULL;
    m_state = 0;
}

QVector2D Skeleton::jointHead() const
{
    return getJoint(nite::JOINT_HEAD);
}

QVector2D Skeleton::jointNeck() const
{
    return getJoint(nite::JOINT_NECK);
}

QVector2D Skeleton::jointLeftShoulder() const
{
    return getJoint(nite::JOINT_LEFT_SHOULDER);
}

QVector2D Skeleton::jointRightShoulder() const
{
    return getJoint(nite::JOINT_RIGHT_SHOULDER);
}

QVector2D Skeleton::jointLeftElbow() const
{
    return getJoint(nite::JOINT_LEFT_ELBOW);
}

QVector2D Skeleton::jointRightElbow() const
{
    return getJoint(nite::JOINT_RIGHT_ELBOW);
}

QVector2D Skeleton::jointLeftHand() const
{
    return getJoint(nite::JOINT_LEFT_HAND);
}

QVector2D Skeleton::jointRightHand() const
{
    return getJoint(nite::JOINT_RIGHT_HAND);
}

QVector2D Skeleton::jointTorso() const
{
    return getJoint(nite::JOINT_TORSO);
}

QVector2D Skeleton::jointLeftHip() const
{
    return getJoint(nite::JOINT_LEFT_HIP);
}

QVector2D Skeleton::jointRightLip() const
{
    return getJoint(nite::JOINT_RIGHT_HIP);
}

QVector2D Skeleton::jointLeftKnee() const
{
    return getJoint(nite::JOINT_LEFT_KNEE);
}

QVector2D Skeleton::jointRightKnee() const
{
    return getJoint(nite::JOINT_RIGHT_KNEE);
}

QVector2D Skeleton::jointLeftFoot() const
{
    return getJoint(nite::JOINT_LEFT_FOOT);
}

QVector2D Skeleton::jointRightFoot() const
{
    return getJoint(nite::JOINT_RIGHT_FOOT);
}

char Skeleton::getState() const
{
    return m_state;
}

QVector2D Skeleton::getJoint(nite::JointType type) const
{
    QVector2D result;

    if (m_skeleton != NULL) {
        float coordinates[2] = {0};
        const nite::SkeletonJoint& joint1 = m_skeleton->getJoint(type);
        m_pUserTracker->convertJointCoordinatesToDepth(joint1.getPosition().x, joint1.getPosition().y, joint1.getPosition().z,
                                                       &coordinates[0], &coordinates[1]);

        coordinates[0]  *= m_width/m_nativeWidth;
        coordinates[1]  *= m_height/m_nativeHeight;

        /*qDebug() << "m_width: " << m_width << ", m_nativeWidth: " << m_nativeWidth;
        qDebug() << "m_heigth: " << m_height << ", m_nativeHeight: " << m_nativeHeight;*/

        result.setX(coordinates[0]);
        result.setY(coordinates[1]);
    }

    return result;
}

void Skeleton::setState(char value)
{
    m_state = value;
    emit jointsChanged();
}

void Skeleton::setSkeleton(const nite::Skeleton& skeleton, nite::UserTracker* pUserTracker)
{
    m_skeleton = const_cast<nite::Skeleton*>(&skeleton);
    m_pUserTracker = pUserTracker;

    if (skeleton.getState() == nite::SKELETON_CALIBRATING) {
        m_state = 1;
    } else if (skeleton.getState() == nite::SKELETON_TRACKED) {
        m_state = 2;
    } else {
        m_state = 0;
    }

    emit jointsChanged();
}

void Skeleton::setNativeResolution(int width, int height)
{
    m_nativeWidth = width;
    m_nativeHeight = height;
}

void Skeleton::setResolution(int width, int height)
{
    m_width = width;
    m_height = height;
}
