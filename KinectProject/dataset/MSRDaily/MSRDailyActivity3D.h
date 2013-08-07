#ifndef MSRDAILYACT3D_H
#define MSRDAILYACT3D_H

#include "dataset/Dataset.h"

namespace dai {

class MSRDailyActivity3D : public Dataset
{
public:
    MSRDailyActivity3D();
    shared_ptr<BaseInstance> getInstance(int activity, int actor, int sample, InstanceType type) const;
};

}

#endif // MSRDAILYACT3D_H
