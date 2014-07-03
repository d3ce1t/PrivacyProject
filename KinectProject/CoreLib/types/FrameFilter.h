#ifndef FRAME_FILTER_H
#define FRAME_FILTER_H

#include "types/DataFrame.h"
#include "types/MaskFrame.h"

namespace dai {

class FrameFilter
{
public:
    FrameFilter();
    void enableFilter(bool value);
    void setMask(shared_ptr<MaskFrame> mask);
    virtual void applyFilter(shared_ptr<DataFrame> frame) = 0;

protected:
    shared_ptr<MaskFrame> m_userMask;
    bool m_enabled;
};

} // End Namespace

#endif // FRAME_FILTER_H
