#ifndef MSR3ACTION3D_H
#define MSR3ACTION3D_H

#include "dataset/Dataset.h"

namespace dai {

class MSR3Action3D : public Dataset
{
public:
    MSR3Action3D();
    shared_ptr<BaseInstance> getInstance(int activity, int actor, int sample, InstanceType type) const;
};

} // End Namespace

#endif // MSR3ACTION3D_H
