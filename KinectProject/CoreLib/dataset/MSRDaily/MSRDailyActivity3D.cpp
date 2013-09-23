#include "MSRDailyActivity3D.h"
#include "MSRDailyDepthInstance.h"
#include "MSRDailySkeletonInstance.h"
#include "MSRDailyColorInstance.h"

namespace dai {

MSRDailyActivity3D::MSRDailyActivity3D()
    : Dataset(":/data/MSRDailyActivity3D.xml")
{
}

shared_ptr<BaseInstance> MSRDailyActivity3D::instance(int activity, int actor, int sample, InstanceType type) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(type, activity, actor, sample);

    switch (type) {
    case INSTANCE_DEPTH:
        return shared_ptr<BaseInstance>(new MSRDailyDepthInstance(instanceInfo));
        break;
    case INSTANCE_SKELETON:
        return shared_ptr<BaseInstance>(new MSRDailySkeletonInstance(instanceInfo));
        break;
    case INSTANCE_COLOR:
        return shared_ptr<BaseInstance>(new MSRDailyColorInstance(instanceInfo));
        break;
    default:
        return nullptr;
    }
}

}
