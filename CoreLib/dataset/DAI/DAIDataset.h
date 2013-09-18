#ifndef DAIDATASET_H
#define DAIDATASET_H

#include "dataset/Dataset.h"

namespace dai {

class DAIDataset : public Dataset
{
public:
    DAIDataset();

protected:
    shared_ptr<BaseInstance> instance(int activity, int actor, int sample, InstanceType type) const;
};

} // End namespace

#endif // DAIDATASET_H
