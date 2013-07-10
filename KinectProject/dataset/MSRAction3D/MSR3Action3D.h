#ifndef MSR3ACTION3D_H
#define MSR3ACTION3D_H

#include "../Dataset.h"
#include "MSRActionDepthInstance.h"
#include "MSRActionSkeletonInstance.h"

namespace dai {

class MSR3Action3D : public Dataset
{
public:
    explicit MSR3Action3D();
    MSRActionDepthInstance* getDepthInstance(int activity, int actor, int sample) const;
    MSRActionSkeletonInstance* getSkeletonInstance(int activity, int actor, int sample) const;
    DataInstance* getColorInstance(int activity, int actor, int sample) const;
    DataInstance* getUserInstance(int activity, int actor, int sample) const;
};

} // End Namespace

#endif // MSR3ACTION3D_H
