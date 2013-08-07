#include "MSR3Action3D.h"
#include "MSRActionDepthInstance.h"
#include "MSRActionSkeletonInstance.h"

namespace dai {

MSR3Action3D::MSR3Action3D()
    : Dataset("/home/jose/Dropbox/DataSet_Descriptor/MSRAction3D.xml")
{
}

shared_ptr<BaseInstance> MSR3Action3D::getInstance(int activity, int actor, int sample, InstanceType type) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(type, activity, actor, sample);

    switch (type) {
    case INSTANCE_DEPTH:
        return shared_ptr<BaseInstance>(new MSRActionDepthInstance(instanceInfo));
        break;
    case INSTANCE_SKELETON:
        return shared_ptr<BaseInstance>(new MSRActionSkeletonInstance(instanceInfo));
        break;
    default:
        return nullptr;
    }
}

} // End namespace
