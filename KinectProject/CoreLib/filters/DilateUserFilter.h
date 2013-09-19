#ifndef DILATEUSERFILTER_H
#define DILATEUSERFILTER_H

#include "types/FrameFilter.h"

namespace dai {

class DilateUserFilter : public FrameFilter
{
public:

#if (defined _MSC_VER)
    DilateUserFilter() {
        m_dilation_size = 15;
    }
#endif

    void applyFilter(shared_ptr<DataFrame> frame);
    void setDilationSize(int value);

private:
    void dilateUserMask(uint8_t* labels);

#if (!defined _MSC_VER)
    int m_dilation_size = 15;
#else
    int m_dilation_size;
#endif
};

} // End Namespace

#endif // DILATEUSERFILTER_H
