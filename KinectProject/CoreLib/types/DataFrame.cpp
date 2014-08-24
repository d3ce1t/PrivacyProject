#include "DataFrame.h"
#include "types/ColorFrame.h"
#include "types/DepthFrame.h"
#include "types/SkeletonFrame.h"
#include "types/MaskFrame.h"
#include "types/MetadataFrame.h"

namespace dai {

shared_ptr<DataFrame> DataFrame::create(FrameType type, int width, int height)
{
    shared_ptr<DataFrame> result = nullptr;

    if (type == Color) {
        result = make_shared<ColorFrame>(width, height);
    }
    else if (type == Depth) {
        result = make_shared<DepthFrame>(width, height);
    }
    else if (type == Skeleton) {
        result = make_shared<SkeletonFrame>();
    }
    else if (type == Mask) {
        result = make_shared<MaskFrame>(width, height);
    }
    else if (type == Metadata) {
        result = make_shared<MetadataFrame>();
    }

    return result;
}

DataFrame::DataFrame(FrameType type)
{
    m_index = -1;
    m_type = type;
}

DataFrame::DataFrame(const DataFrame& other)
{
    m_index = other.m_index;
    m_type = other.m_type;
}

DataFrame& DataFrame::operator=(const DataFrame& other)
{
    m_index = other.m_index;
    m_type = other.m_type;
    return *this;
}

DataFrame::FrameType DataFrame::getType() const
{
    return m_type;
}

unsigned int DataFrame::getIndex() const
{
    return m_index;
}

void DataFrame::setIndex(unsigned int index)
{
    m_index = index;
}

} // End Namespace
