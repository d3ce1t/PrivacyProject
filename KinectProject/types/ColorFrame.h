#ifndef COLOR_FRAME_H
#define COLOR_FRAME_H

#include "GenericFrame.h"
#include <stdint.h>

namespace dai {

struct RGBColor {
    u_int8_t red;
    u_int8_t green;
    u_int8_t blue;
};

class ColorFrame : public GenericFrame<RGBColor>
{
public:
    // Constructors
    ColorFrame();
    ColorFrame(int width, int height);
    virtual ~ColorFrame();
    ColorFrame(const ColorFrame& other);

    // Overriden operators
    ColorFrame& operator=(const ColorFrame& other);
};

} // End Namespace

#endif // COLOR_FRAME_H
