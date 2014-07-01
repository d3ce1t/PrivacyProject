#include "MSRDailyActivity3D.h"
#include "MSRDailyDepthInstance.h"
#include "MSRDailySkeletonInstance.h"
#include "MSRDailyColorInstance.h"

namespace dai {

MSRDailyActivity3D::MSRDailyActivity3D()
    : Dataset(":/data/MSRDailyActivity3D.xml")
{
}

shared_ptr<StreamInstance> MSRDailyActivity3D::instance(int activity, int actor, int sample, DataFrame::FrameType type) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(type, activity, actor, sample);

    switch (type) {
    case DataFrame::Depth:
        return shared_ptr<StreamInstance>(new MSRDailyDepthInstance(instanceInfo));
        break;
    case DataFrame::Skeleton:
        return shared_ptr<StreamInstance>(new MSRDailySkeletonInstance(instanceInfo));
        break;
    case DataFrame::Color:
        return shared_ptr<StreamInstance>(new MSRDailyColorInstance(instanceInfo));
        break;
    default:
        return nullptr;
    }
}

}
