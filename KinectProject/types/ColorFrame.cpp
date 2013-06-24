#include "ColorFrame.h"

namespace dai {

ColorFrame::ColorFrame()
    : GenericFrame<RGBAColor>(DataFrame::Color)
{
}

ColorFrame::ColorFrame(int width, int height)
    : GenericFrame<RGBAColor>(width, height, DataFrame::Color)
{
}

ColorFrame::~ColorFrame()
{
}

ColorFrame::ColorFrame(const ColorFrame& other)
    : GenericFrame<RGBAColor>(other)
{
}

ColorFrame& ColorFrame::operator=(const ColorFrame& other)
{
    GenericFrame<RGBAColor>::operator=(other);
    return *this;
}


} // End Namespace


