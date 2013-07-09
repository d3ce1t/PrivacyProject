#include "ColorFrame.h"

namespace dai {

ColorFrame::ColorFrame()
    : GenericFrame<RGBColor>(DataFrame::Color)
{
}

ColorFrame::ColorFrame(int width, int height)
    : GenericFrame<RGBColor>(width, height, DataFrame::Color)
{
}

ColorFrame::~ColorFrame()
{
}

ColorFrame::ColorFrame(const ColorFrame& other)
    : GenericFrame<RGBColor>(other)
{
}

ColorFrame& ColorFrame::operator=(const ColorFrame& other)
{
    GenericFrame<RGBColor>::operator=(other);
    return *this;
}


} // End Namespace


