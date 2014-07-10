#ifndef COLOR_FRAME_H
#define COLOR_FRAME_H

#include "GenericFrame.h"
#include <stdint.h>

namespace dai {

struct RGBColor {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    //u_int8_t alpha; // padding
};

typedef GenericFrame<RGBColor, DataFrame::Color> ColorFrame;

} // End Namespace

#endif // COLOR_FRAME_H
