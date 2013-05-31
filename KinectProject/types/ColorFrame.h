#ifndef COLOR_FRAME_H
#define COLOR_FRAME_H

#include "GenericFrame.h"

namespace dai {

struct RGBAColor {
    float red;
    float green;
    float blue;
    float alpha;
};

class ColorFrame : public GenericFrame<RGBAColor>
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
