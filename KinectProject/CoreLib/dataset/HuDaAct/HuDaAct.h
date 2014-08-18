#ifndef HUDAACT_H
#define HUDAACT_H

#include "dataset/Dataset.h"

namespace dai {

class HuDaAct : public Dataset
{
public:
    HuDaAct();

protected:
    shared_ptr<StreamInstance> instance(int actor, int camera, int sample, const QList<QString>& label, DataFrame::FrameType type) const override;
};

} // End Namespace

#endif // HUDAACT_H
