#include "MaskFrame.h"

namespace dai {

MaskFrame::MaskFrame()
    : GenericFrame<uint8_t>(DataFrame::Mask)
{ 
}

MaskFrame::MaskFrame(int width, int height)
    : GenericFrame<uint8_t>(width, height, DataFrame::Mask)
{
}

MaskFrame::~MaskFrame()
{
}

MaskFrame::MaskFrame(const MaskFrame& other)
    : GenericFrame<uint8_t>(other)
{

}

MaskFrame& MaskFrame::operator=(const MaskFrame& other)
{
    GenericFrame<uint8_t>::operator=(other);
    return *this;
}

} // End Namespace
