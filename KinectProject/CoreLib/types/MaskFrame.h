#ifndef USERFRAME_H
#define USERFRAME_H

#include "GenericFrame.h"

namespace dai {

class MaskFrame : public GenericFrame<uint8_t>
{
public:
    // Constructors
    MaskFrame();
    MaskFrame(int width, int height);
    virtual ~MaskFrame();
    MaskFrame(const MaskFrame& other);

    // Overriden operators
    MaskFrame& operator=(const MaskFrame& other);
};

} // End Namespace

#endif // USERFRAME_H
