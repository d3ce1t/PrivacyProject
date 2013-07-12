#include "FrameFilter.h"

namespace dai {

FrameFilter::FrameFilter()
{
    m_userMask = NULL;
    m_enabled = true;
}

void FrameFilter::setMask(UserFrame* mask)
{
    m_userMask = mask;
}

void FrameFilter::enableFilter(bool value)
{
    m_enabled = value;
}

} // End Namespace
