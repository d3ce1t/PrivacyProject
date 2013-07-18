#ifndef DAIDATASET_H
#define DAIDATASET_H

#include "dataset/Dataset.h"
#include "DAIDepthInstance.h"
#include "DAIColorInstance.h"
#include "DAIUserInstance.h"

namespace dai {

class DAIDataset : public Dataset
{
public:
    explicit DAIDataset();
    shared_ptr<DataInstance> getDepthInstance(int activity, int actor, int sample) const;
    shared_ptr<DataInstance> getSkeletonInstance(int activity, int actor, int sample) const;
    shared_ptr<DataInstance> getColorInstance(int activity, int actor, int sample) const;
    shared_ptr<DataInstance> getUserInstance(int activity, int actor, int sample) const;
};

} // End namespace

#endif // DAIDATASET_H
