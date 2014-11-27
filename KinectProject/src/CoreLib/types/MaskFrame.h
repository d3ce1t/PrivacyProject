#ifndef MASK_FRAME_H
#define MASK_FRAME_H

#include "GenericFrame.h"

namespace dai {

typedef GenericFrame<uint8_t, DataFrame::Mask> MaskFrame;
typedef shared_ptr<MaskFrame> MaskFramePtr;


} // End Namespace

#endif // MASK_FRAME_H
