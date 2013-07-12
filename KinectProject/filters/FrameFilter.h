#ifndef FRAME_FILTER_H
#define FRAME_FILTER_H

#include "types/DataFrame.h"
#include "types/UserFrame.h"

namespace dai {

class FrameFilter
{
public:
    FrameFilter();
    void setMask(UserFrame *mask);
    void enableFilter(bool value);
    virtual void applyFilter(DataFrame* frame) = 0;

protected:
    UserFrame* m_userMask;
    bool m_enabled;
};

} // End Namespace

#endif // FRAME_FILTER_H
