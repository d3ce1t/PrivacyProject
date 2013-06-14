#ifndef DAIDATASET_H
#define DAIDATASET_H

#include "Dataset.h"
#include "DAIDepthInstance.h"
#include "DAIColorInstance.h"

namespace dai {

class DAIDataset : public Dataset
{
public:
    explicit DAIDataset();
    DAIDepthInstance* getDepthInstance(int activity, int actor, int sample);
    DataInstance* getSkeletonInstance(int activity, int actor, int sample);
    DAIColorInstance* getColorInstance(int activity, int actor, int sample);
};

} // End namespace

#endif // DAIDATASET_H
