#include "SkeletonFrame.h"

namespace dai {

SkeletonFrame::SkeletonFrame()
    : DataFrame(DataFrame::Skeleton)
{
    m_width = 0;
    m_height = 0;
}

SkeletonFrame::SkeletonFrame(int width, int height)
    : DataFrame(DataFrame::Skeleton)
{
    m_width = width;
    m_height = height;
}

SkeletonFrame::SkeletonFrame(const SkeletonFrame& other)
    : DataFrame(other)
{
    m_hashSkeletons = other.m_hashSkeletons; // implicit sharing
    m_width = other.m_width;
    m_height = other.m_height;
}

SkeletonFrame& SkeletonFrame::operator=(const SkeletonFrame& other)
{
    DataFrame::operator=(other);
    m_hashSkeletons = other.m_hashSkeletons;
    m_width = other.m_width;
    m_height = other.m_height;
    return *this;
}

shared_ptr<DataFrame> SkeletonFrame::clone() const
{
    return shared_ptr<DataFrame>(new SkeletonFrame(*this));
}

SkeletonPtr SkeletonFrame::getSkeleton(int userId) const
{
    return m_hashSkeletons.value(userId);;
}

QList<SkeletonPtr> SkeletonFrame::skeletons() const
{
    return m_hashSkeletons.values();
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

SkeletonFramePtr SkeletonFrame::fromBinary(const QByteArray& buffer)
{
    QByteArray bufferTmp = buffer;
    SkeletonFramePtr skeletonFrame = make_shared<SkeletonFrame>();

    int bytes_read = 0;
    int num_skeletons = bufferTmp[0];
    bufferTmp = bufferTmp.remove(0, 1);

    for (int i=0; i<num_skeletons; ++i)
    {
        SkeletonPtr skeleton = dai::Skeleton::fromBinary(bufferTmp, &bytes_read);
        skeletonFrame->setSkeleton(i+1, skeleton);
        bufferTmp.remove(0, bytes_read);
    }

    return skeletonFrame;
}

QByteArray SkeletonFrame::toBinary() const
{
    QByteArray buffer;

    // Header
    buffer.append(m_hashSkeletons.size()); // Number of skeletons

    // Body
    for (SkeletonPtr skeleton : m_hashSkeletons)
    {
        QByteArray skeletonBin = skeleton->toBinary();
        buffer.append(skeletonBin);
    }

    return buffer;
}

} // End Namespace
