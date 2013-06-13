#include "SkeletonVector.h"

namespace dai {

SkeletonVector::SkeletonVector()
{
    m_length = 0;
}

SkeletonVector::SkeletonVector(SkeletonJoint* p1, SkeletonJoint* p2)
{
    setVector(p1, p2);
}

void SkeletonVector::setVector(SkeletonJoint* p1, SkeletonJoint* p2)
{
    // Here p1 is the vertex
    m_vector.setX(p2->getPosition().x() - p1->getPosition().x());
    m_vector.setY(p2->getPosition().y() - p1->getPosition().y());
    m_vector.setZ(p2->getPosition().z() - p1->getPosition().z());
    m_length = m_vector.length();
    m_vector.normalize();
    m_joint1 = p1;
    m_joint2 = p2;
}

const QVector3D& SkeletonVector::vector() const
{
    return m_vector;
}

float SkeletonVector::length() const
{
    return m_length;
}

const SkeletonJoint& SkeletonVector::joint1() const
{
    return *m_joint1;
}

const SkeletonJoint& SkeletonVector::joint2() const
{
    return *m_joint2;
}

} // End Namespace
