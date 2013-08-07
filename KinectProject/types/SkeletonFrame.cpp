#include "SkeletonFrame.h"

namespace dai {

SkeletonFrame::SkeletonFrame()
    : DataFrame(DataFrame::Skeleton)
{
}

SkeletonFrame::SkeletonFrame(const SkeletonFrame& other)
    : DataFrame(other)
{
    m_hashSkeletons = other.m_hashSkeletons; // implicit sharing
}

SkeletonFrame& SkeletonFrame::operator=(const SkeletonFrame& other)
{
    DataFrame::operator=(other);
    m_hashSkeletons = other.m_hashSkeletons;
    return *this;
}

shared_ptr<DataFrame> SkeletonFrame::clone() const
{
    return shared_ptr<DataFrame>(new SkeletonFrame(*this));
}

shared_ptr<dai::Skeleton> SkeletonFrame::getSkeleton(int userId)
{
    return m_hashSkeletons.value(userId);;
}

void SkeletonFrame::setSkeleton(int userId, shared_ptr<dai::Skeleton> skeleton)
{
    m_hashSkeletons.insert(userId, skeleton);
}

QList<int> SkeletonFrame::getAllUsersId() const
{
    return m_hashSkeletons.keys();
}

void SkeletonFrame::clear()
{
    m_hashSkeletons.clear();
}

void SkeletonFrame::write(QFile &of) const
{
    // Write frame index
    of.write( (char*) &m_index, sizeof(m_index) );

    // Write Num Skeletons
    int numSkeletons = m_hashSkeletons.size();
    of.write( (char*) &numSkeletons, sizeof(numSkeletons) );

    // Write all skeletons
    foreach (int key, m_hashSkeletons.keys())
    {
        auto skeleton = m_hashSkeletons.value(key);

        // Write user key
        of.write( (char*) &key, sizeof(key) );

        // Write Skeleton Type (determine used joints)
        Skeleton::SkeletonType type = skeleton->getType();
        of.write( (char*) &type, sizeof(Skeleton::SkeletonType) );

        // Write all joints (by now, used and unused)
        for (int i=0; i<MAX_JOINTS; ++i) {
            const SkeletonJoint& joint = skeleton->getJoint( (SkeletonJoint::JointType) i);
            const Point3f& position = joint.getPosition();
            of.write( (char*) position.dataPtr(), 3 * sizeof(double) );
        }
    }

    of.flush();
}

} // End Namespace
