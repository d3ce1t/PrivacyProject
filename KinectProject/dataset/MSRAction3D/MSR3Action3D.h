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
    shared_ptr<DataInstance> getDepthInstance(int activity, int actor, int sample) const;
    shared_ptr<DataInstance> getSkeletonInstance(int activity, int actor, int sample) const;
    shared_ptr<DataInstance> getColorInstance(int activity, int actor, int sample) const;
    shared_ptr<DataInstance> getUserInstance(int activity, int actor, int sample) const;
};

} // End Namespace

#endif // MSR3ACTION3D_H
