#ifndef DILATEUSERFILTER_H
#define DILATEUSERFILTER_H

#include "FrameFilter.h"

namespace dai {

class DilateUserFilter : public FrameFilter
{
public:
    void applyFilter(shared_ptr<DataFrame> frame);

private:
    void dilateUserMask(u_int8_t* labels);
};

} // End Namespace

#endif // DILATEUSERFILTER_H
