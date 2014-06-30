#ifndef MSRDAILYACT3D_H
#define MSRDAILYACT3D_H

#include "dataset/Dataset.h"

namespace dai {

class MSRDailyActivity3D : public Dataset
{
public:
    MSRDailyActivity3D();

protected:
    shared_ptr<BaseInstance> instance(int activity, int actor, int sample, DataFrame::FrameType type) const;
};

}

#endif // MSRDAILYACT3D_H
