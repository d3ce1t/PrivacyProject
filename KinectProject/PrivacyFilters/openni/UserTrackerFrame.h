#ifndef USERTRACKERFRAME_H
#define USERTRACKERFRAME_H

#include "types/SkeletonFrame.h"
#include "types/UserFrame.h"
#include "types/DepthFrame.h"

namespace dai {

class UserTrackerFrame
{
public:
    UserTrackerFrame();
    ~UserTrackerFrame() = default;
    UserTrackerFrame(int width, int height);
    UserTrackerFrame(const UserTrackerFrame& other);

    // Overload operators
    UserTrackerFrame& operator=(const UserTrackerFrame& other);

    shared_ptr<SkeletonFrame> skeletonFrame;
    shared_ptr<UserFrame> userFrame;
    shared_ptr<DepthFrame> depthFrame;
};

} // End Namespace

#endif // USERTRACKERFRAME_H
