#ifndef DAI4REID_H
#define DAI4REID_H

#include "dataset/Dataset.h"

namespace dai {

class DAI4REID : public Dataset
{
public:
    DAI4REID();

protected:
    shared_ptr<StreamInstance> instance(int actor, int camera, int sample, const QList<QString>& label, DataFrame::FrameType type) const override;
};

} // End Namespace

#endif // DAI4REID_H
