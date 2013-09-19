#ifndef INVISIBILITY_FILTER_H
#define INVISIBILITY_FILTER_H

#include "types/FrameFilter.h"
#include "types/ColorFrame.h"

namespace dai {

class InvisibilityFilter : public FrameFilter
{
public:
    InvisibilityFilter();
    void applyFilter(shared_ptr<DataFrame> frame);

private:
    ColorFrame m_background;
};

} // End namespace

#endif // INVISIBILITY_FILTER_H
