#include "DataFrame.h"

namespace dai {

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
