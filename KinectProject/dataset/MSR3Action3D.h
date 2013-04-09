#ifndef MSR3ACTION3D_H
#define MSR3ACTION3D_H

#include "Dataset.h"
#include "MSRActionDepthInstance.h"

namespace dai {

class MSR3Action3D : public Dataset
{
public:
    explicit MSR3Action3D();
    MSRActionDepthInstance* getDepthInstance(int activity, int actor, int sample);
    DataInstance& getColorInstance(int activity, int actor, int sample) {}
};

} // End Namespace

#endif // MSR3ACTION3D_H
