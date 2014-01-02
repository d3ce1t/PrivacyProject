#include "UserTrackerFrame.h"

namespace dai {

UserTrackerFrame::UserTrackerFrame()
    : DataFrame(DataFrame::UserTracker)
{
    skeletonFrame.reset(new SkeletonFrame);
    userFrame.reset(new UserFrame);
}

UserTrackerFrame::UserTrackerFrame(int width, int height)
    : DataFrame(DataFrame::UserTracker)
{
    skeletonFrame.reset(new SkeletonFrame);
    userFrame.reset(new UserFrame(width, height));
}

UserTrackerFrame::UserTrackerFrame(const UserTrackerFrame& other)
    : DataFrame(other)
{
    skeletonFrame = other.skeletonFrame;
    userFrame = other.userFrame;
}

UserTrackerFrame& UserTrackerFrame::operator=(const UserTrackerFrame& other)
{
    DataFrame::operator=(other);
    skeletonFrame = other.skeletonFrame;
    userFrame = other.userFrame;
    return *this;
}

shared_ptr<DataFrame> UserTrackerFrame::clone() const
{
    return shared_ptr<DataFrame>(new UserTrackerFrame(*this));
}

} // End Namespace
