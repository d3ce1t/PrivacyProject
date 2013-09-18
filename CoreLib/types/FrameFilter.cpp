#include "FrameFilter.h"

namespace dai {

FrameFilter::FrameFilter()
{
    m_userMask = nullptr;
    m_enabled = false;
}

void FrameFilter::setMask(shared_ptr<UserFrame> mask)
{
    m_userMask = mask;
}

void FrameFilter::enableFilter(bool value) {
    m_enabled = value;
}

} // End Namespace
