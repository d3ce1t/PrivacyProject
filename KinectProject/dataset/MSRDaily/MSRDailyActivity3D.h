#ifndef MSRDAILYACT3D_H
#define MSRDAILYACT3D_H

#include "dataset/Dataset.h"
#include "MSRDailyDepthInstance.h"
#include "MSRDailySkeletonInstance.h"
#include <iostream>

using namespace std;

namespace dai {

class MSRDailyActivity3D : public Dataset
{
public:
    explicit MSRDailyActivity3D();
    shared_ptr<DataInstance> getDepthInstance(int activity, int actor, int sample) const;
    shared_ptr<DataInstance> getSkeletonInstance(int activity, int actor, int sample) const;
    shared_ptr<DataInstance> getColorInstance(int activity, int actor, int sample) const;
    shared_ptr<DataInstance> getUserInstance(int activity, int actor, int sample) const;
};

}

#endif // MSRDAILYACT3D_H
