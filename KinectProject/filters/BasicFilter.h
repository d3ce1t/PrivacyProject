#ifndef BASICFILTER_H
#define BASICFILTER_H

#include "filters/FrameFilter.h"
#include "types/ColorFrame.h"

namespace dai {

class BasicFilter : public FrameFilter
{
public:
    BasicFilter();
    void applyFilter(DataFrame* frame);

private:
    ColorFrame m_background;
};

} // End namespace

#endif // BASICFILTER_H
