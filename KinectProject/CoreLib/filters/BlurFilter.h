#ifndef BLURFILTER_H
#define BLURFILTER_H

#include "types/FrameFilter.h"

namespace dai {

class BlurFilter : public FrameFilter
{
public:
    void applyFilter(shared_ptr<DataFrame> frame);
};

} // End Namespace

#endif // BLURFILTER_H
