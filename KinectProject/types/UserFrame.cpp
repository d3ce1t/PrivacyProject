#include "UserFrame.h"

namespace dai {

UserFrame::UserFrame()
    : GenericFrame<u_int8_t>(DataFrame::User)
{ 
}

UserFrame::UserFrame(int width, int height)
    : GenericFrame<u_int8_t>(width, height, DataFrame::User)
{
}

UserFrame::~UserFrame()
{
}

UserFrame::UserFrame(const UserFrame& other)
    : GenericFrame<u_int8_t>(other)
{

}

UserFrame& UserFrame::operator=(const UserFrame& other)
{
    GenericFrame<u_int8_t>::operator=(other);
    return *this;
}

} // End Namespace
