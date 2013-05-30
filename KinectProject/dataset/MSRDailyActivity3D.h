#ifndef MSRDAILYACT3D_H
#define MSRDAILYACT3D_H

#include "Dataset.h"
#include "MSRDailyDepthInstance.h"
#include "MSRDailySkeletonInstance.h"
#include <iostream>

using namespace std;

namespace dai {

class MSRDailyActivity3D : public Dataset
{
public:
    explicit MSRDailyActivity3D();
    MSRDailyDepthInstance* getDepthInstance(int activity, int actor, int sample);
    MSRDailySkeletonInstance* getSkeletonInstance(int activity, int actor, int sample);
    DataInstance* getColorInstance(int activity, int actor, int sample);
};

}

#endif // MSRDAILYACT3D_H
