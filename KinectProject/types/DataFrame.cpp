#include "DataFrame.h"

namespace dai {

DataFrame::DataFrame()
{
    m_index = -1;   
}

DataFrame::DataFrame(const DataFrame& other)
{
    m_index = other.m_index;
}

DataFrame& DataFrame::operator=(const DataFrame& other)
{
    m_index = other.m_index;
    return *this;
}

int DataFrame::getIndex() const
{
    return m_index;
}

void DataFrame::setIndex(int index)
{
    m_index = index;
}

} // End Namespace
