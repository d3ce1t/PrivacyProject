#ifndef DAIDATASET_H
#define DAIDATASET_H

#include "Dataset.h"
#include "DAIDepthInstance.h"

namespace dai {

class DAIDataset : public Dataset
{
public:
    explicit DAIDataset();
    DAIDepthInstance* getDepthInstance(int activity, int actor, int sample);
    DataInstance* getSkeletonInstance(int activity, int actor, int sample);
    DataInstance* getColorInstance(int activity, int actor, int sample);
};

} // End namespace

#endif // DAIDATASET_H
