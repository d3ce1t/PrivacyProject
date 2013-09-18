#include "UserFrame.h"

namespace dai {

UserFrame::UserFrame()
    : GenericFrame<uint8_t>(DataFrame::User)
{ 
}

UserFrame::UserFrame(int width, int height)
    : GenericFrame<uint8_t>(width, height, DataFrame::User)
{
}

UserFrame::~UserFrame()
{
}

UserFrame::UserFrame(const UserFrame& other)
    : GenericFrame<uint8_t>(other)
{

}

UserFrame& UserFrame::operator=(const UserFrame& other)
{
    GenericFrame<uint8_t>::operator=(other);
    return *this;
}

} // End Namespace
