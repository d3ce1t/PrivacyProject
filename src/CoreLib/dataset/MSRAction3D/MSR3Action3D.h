#ifndef MSR3ACTION3D_H
#define MSR3ACTION3D_H

#include "dataset/Dataset.h"

namespace dai {

class MSR3Action3D : public Dataset
{
public:
    MSR3Action3D();

protected:
    shared_ptr<StreamInstance> instance(int actor, int camera, int sample, const QList<QString>& label, DataFrame::FrameType type) const override;
};

} // End Namespace

#endif // MSR3ACTION3D_H
