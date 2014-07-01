#include "MSR3Action3D.h"
#include "MSRActionDepthInstance.h"
#include "MSRActionSkeletonInstance.h"

namespace dai {

MSR3Action3D::MSR3Action3D()
    : Dataset(":/data/MSRAction3D.xml")
{
}

shared_ptr<StreamInstance> MSR3Action3D::instance(int activity, int actor, int sample, DataFrame::FrameType type) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(type, activity, actor, sample);

    switch (type) {
    case DataFrame::Depth:
        return shared_ptr<StreamInstance>(new MSRActionDepthInstance(instanceInfo));
        break;
    case DataFrame::Skeleton:
        return shared_ptr<StreamInstance>(new MSRActionSkeletonInstance(instanceInfo));
        break;
    default:
        return nullptr;
    }
}

} // End namespace
