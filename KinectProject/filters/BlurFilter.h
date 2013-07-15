#ifndef BLURFILTER_H
#define BLURFILTER_H

#include "FrameFilter.h"

namespace dai {

class BlurFilter : public FrameFilter
{
public:
    void applyFilter(DataFrame* frame);
};

} // End Namespace

#endif // BLURFILTER_H
