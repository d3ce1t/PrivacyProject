#include "UserFrame.h"

namespace dai {

UserFrame::UserFrame()
    : GenericFrame<short int>(DataFrame::User)
{ 
}

UserFrame::UserFrame(int width, int height)
    : GenericFrame<short int>(width, height, DataFrame::User)
{
}

UserFrame::~UserFrame()
{
}

UserFrame::UserFrame(const UserFrame& other)
    : GenericFrame<short int>(other)
{

}

UserFrame& UserFrame::operator=(const UserFrame& other)
{
    GenericFrame<short int>::operator=(other);
    return *this;
}

} // End Namespace
