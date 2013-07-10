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
    DAIDepthInstance* getDepthInstance(int activity, int actor, int sample) const;
    DataInstance* getSkeletonInstance(int activity, int actor, int sample) const;
    DAIColorInstance* getColorInstance(int activity, int actor, int sample) const;
    DAIUserInstance* getUserInstance(int activity, int actor, int sample) const;
};

} // End namespace

#endif // DAIDATASET_H
