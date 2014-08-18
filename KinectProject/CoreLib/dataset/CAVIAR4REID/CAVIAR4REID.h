#ifndef CAVIAR4REID_H
#define CAVIAR4REID_H

#include "dataset/Dataset.h"

namespace dai {

class CAVIAR4REID : public Dataset
{
public:
    CAVIAR4REID();

protected:
    shared_ptr<StreamInstance> instance(int actor, int camera, int sample, const QList<QString>& label, DataFrame::FrameType type) const;
};

}

#endif // CAVIAR4REID_H
