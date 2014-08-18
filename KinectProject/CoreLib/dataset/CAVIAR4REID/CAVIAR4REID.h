#ifndef CAVIAR4REID_H
#define CAVIAR4REID_H

#include "dataset/Dataset.h"

namespace dai {

class CAVIAR4REID : public Dataset
{
public:
    CAVIAR4REID();

protected:
    shared_ptr<StreamInstance> instance(int activity, int actor, int sample, DataFrame::FrameType type) const;
};

}

#endif // CAVIAR4REID_H
