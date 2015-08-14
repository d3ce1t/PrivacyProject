#ifndef IASLAB_RGBD_ID_H
#define IASLAB_RGBD_ID_H

#include "dataset/Dataset.h"

namespace dai {

class IASLAB_RGBD_ID : public Dataset
{
public:
    IASLAB_RGBD_ID();

protected:
    shared_ptr<StreamInstance> instance(int actor, int camera, int sample, const QList<QString>& label, DataFrame::FrameType type) const;
};

}

#endif // IASLAB_RGBD_ID_H
