#include "MSRDailyActivity3D.h"
#include "MSRDailyDepthInstance.h"
#include "MSRDailySkeletonInstance.h"
#include "MSRDailyColorInstance.h"

namespace dai {

MSRDailyActivity3D::MSRDailyActivity3D()
    : Dataset(":/data/MSRDailyActivity3D.xml")
{
}

shared_ptr<StreamInstance> MSRDailyActivity3D::instance(int actor, int camera, int sample, const QList<QString> &label, DataFrame::FrameType type) const
{
    const shared_ptr<InstanceInfo> instanceInfo = m_metadata->instance(actor, camera, sample, label, type);

    switch (type) {
    case DataFrame::Depth:
        return make_shared<MSRDailyDepthInstance>(*instanceInfo);
        break;
    case DataFrame::Skeleton:
        return make_shared<MSRDailySkeletonInstance>(*instanceInfo);
        break;
    case DataFrame::Color:
        return make_shared<MSRDailyColorInstance>(*instanceInfo);
        break;
    default:
        return nullptr;
    }
}

}
