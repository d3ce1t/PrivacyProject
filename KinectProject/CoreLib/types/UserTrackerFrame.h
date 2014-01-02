#ifndef USERTRACKERFRAME_H
#define USERTRACKERFRAME_H

#include "types/DataFrame.h"
#include "types/SkeletonFrame.h"
#include "types/UserFrame.h"

namespace dai {

class UserTrackerFrame : public DataFrame
{
public:
    UserTrackerFrame();
    UserTrackerFrame(int width, int height);
    UserTrackerFrame(const UserTrackerFrame& other);
    shared_ptr<DataFrame> clone() const override;

    // Overload operators
    UserTrackerFrame& operator=(const UserTrackerFrame& other);

    shared_ptr<SkeletonFrame> skeletonFrame;
    shared_ptr<UserFrame> userFrame;
};

} // End Namespace

#endif // USERTRACKERFRAME_H
