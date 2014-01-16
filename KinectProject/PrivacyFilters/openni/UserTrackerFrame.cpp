#include "UserTrackerFrame.h"

namespace dai {

UserTrackerFrame::UserTrackerFrame()
{
    skeletonFrame.reset(new SkeletonFrame);
    userFrame.reset(new UserFrame);
    depthFrame.reset(new DepthFrame);
}

UserTrackerFrame::UserTrackerFrame(int width, int height)
{
    skeletonFrame.reset(new SkeletonFrame);
    userFrame.reset(new UserFrame(width, height));
    depthFrame.reset(new DepthFrame(width, height));
}

UserTrackerFrame::UserTrackerFrame(const UserTrackerFrame& other)
{
    skeletonFrame = other.skeletonFrame;
    userFrame = other.userFrame;
    depthFrame = other.depthFrame;
}

UserTrackerFrame& UserTrackerFrame::operator=(const UserTrackerFrame& other)
{
    skeletonFrame = other.skeletonFrame;
    userFrame = other.userFrame;
    depthFrame = other.depthFrame;
    return *this;
}

} // End Namespace
