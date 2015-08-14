#include "IASLAB_RGBD_ID.h"
#include "IASLAB_RGBD_ID_Instance.h"

namespace dai {

IASLAB_RGBD_ID::IASLAB_RGBD_ID()
    : Dataset(":/data/IASLAB_RGBD_ID.xml")
{
}

shared_ptr<StreamInstance> IASLAB_RGBD_ID::instance(int actor, int camera, int sample, const QList<QString> &label, DataFrame::FrameType type) const
{
    const shared_ptr<InstanceInfo> instanceInfo = m_metadata->instance(actor, camera, sample, label, type);

    if (type == DataFrame::Color || type == DataFrame::Depth || type == DataFrame::Mask || type == DataFrame::Skeleton)
    {
         return make_shared<IASLAB_RGBD_ID_Instance>(*instanceInfo);
    }

    return nullptr;
}

} // End Namespace
