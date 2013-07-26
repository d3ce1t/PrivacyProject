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
    shared_ptr<dai::Skeleton> skeleton;

    if (m_hashSkeletons.contains(userId))
        skeleton = m_hashSkeletons.value(userId);
    else {
        skeleton.reset(new dai::Skeleton);
        m_hashSkeletons.insert(userId, skeleton);
    }

    return skeleton;
}

/*void SkeletonFrame::setSkeleton(int userId, const shared_ptr<dai::Skeleton> skeleton)
{
    m_hashSkeletons.insert(userId, skeleton);
}*/

QList<int> SkeletonFrame::getAllUsersId() const
{
    return m_hashSkeletons.keys();
}

void SkeletonFrame::clear()
{
    m_hashSkeletons.clear();
}

} // End Namespace
