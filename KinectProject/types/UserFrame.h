#ifndef USERFRAME_H
#define USERFRAME_H

#include "GenericFrame.h"

namespace dai {

class UserFrame : public GenericFrame<u_int8_t>
{
public:
    // Constructors
    UserFrame();
    UserFrame(int width, int height);
    virtual ~UserFrame();
    UserFrame(const UserFrame& other);

    // Overriden operators
    UserFrame& operator=(const UserFrame& other);
};

} // End Namespace

#endif // USERFRAME_H
