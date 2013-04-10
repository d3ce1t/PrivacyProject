#include "Skeleton.h"
#include <limits>
#include <QDebug>

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

QVector3D Skeleton::maxValue(const Skeleton& frame)
{
    QHashIterator<SkeletonJoint::JointType, SkeletonJoint*> it(frame.m_joints);

    float bestXValue = std::numeric_limits<int>::min();
    float bestYValue = std::numeric_limits<int>::min();
    float bestZValue = std::numeric_limits<int>::min();

    while (it.hasNext())
    {
        it.next();
        SkeletonJoint* joint = it.value();

        float tempX = joint->getPosition().x();
        float tempY = joint->getPosition().y();
        float tempZ = joint->getPosition().z();

        if(tempX > bestXValue)
        {
            bestXValue = tempX;
        }

        if(tempY > bestYValue)
        {
            bestYValue = tempY;
        }

        if(tempZ > bestZValue)
        {
            bestZValue = tempZ;
        }
    }

    if (bestXValue >= 1)
        qDebug() << "MaxX: " << bestXValue;

    if (bestYValue >= 1)
        qDebug() << "MaxY: " << bestYValue;

    return QVector3D(bestXValue, bestYValue, bestZValue);
}

QVector3D Skeleton::minValue(const Skeleton &frame)
{
    QHashIterator<SkeletonJoint::JointType, SkeletonJoint*> it(frame.m_joints);

    float bestXValue = std::numeric_limits<int>::max();
    float bestYValue = std::numeric_limits<int>::max();
    float bestZValue = std::numeric_limits<int>::max();

    while (it.hasNext())
    {
        it.next();
        SkeletonJoint* joint = it.value();

        float tempX = joint->getPosition().x();
        float tempY = joint->getPosition().y();
        float tempZ = joint->getPosition().z();

        if(tempX < bestXValue)
        {
            bestXValue = tempX;
        }

        if(tempY < bestYValue)
        {
            bestYValue = tempY;
        }

        if(tempZ < bestZValue)
        {
            bestZValue = tempZ;
        }
    }

    if (bestXValue <= -1)
        qDebug() << "Min: " << bestXValue;

    if (bestYValue <= -1)
        qDebug() << "MinY: " << bestYValue;

    return QVector3D(bestXValue, bestYValue, bestZValue);
}

} // End Namespace
