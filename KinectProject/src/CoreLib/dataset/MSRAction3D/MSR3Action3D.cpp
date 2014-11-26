#include "MSR3Action3D.h"
#include "MSRActionDepthInstance.h"
#include "MSRActionSkeletonInstance.h"

namespace dai {

MSR3Action3D::MSR3Action3D()
    : Dataset(":/data/MSRAction3D.xml")
{
}

shared_ptr<StreamInstance> MSR3Action3D::instance(int actor, int camera, int sample, const QList<QString> &label, DataFrame::FrameType type) const
{
    const shared_ptr<InstanceInfo> instanceInfo = m_metadata->instance(actor, camera, sample, label, type);

    switch (type) {
    case DataFrame::Depth:
        return make_shared<MSRActionDepthInstance>(*instanceInfo);
        break;
    case DataFrame::Skeleton:
        return make_shared<MSRActionSkeletonInstance>(*instanceInfo);
        break;
    default:
        return nullptr;
    }
}

} // End namespace
