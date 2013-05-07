#include "MSRDailyActivity3D.h"
#include "MSRDailyDepthInstance.h"

namespace dai {

MSRDailyActivity3D::MSRDailyActivity3D()
    : Dataset("/home/jose/Dropbox/DataSet_Descriptor/MSRDailyActivity3D.xml")
{
}

MSRDailyDepthInstance *MSRDailyActivity3D::getDepthInstance(int activity, int actor, int sample)
{
    MSRDailyDepthInstance* result = 0;
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Depth, activity, actor, sample);
    result = new MSRDailyDepthInstance(instanceInfo);
    return result;
}

MSRDailySkeletonInstance* MSRDailyActivity3D::getSkeletonInstance(int activity, int actor, int sample)
{
    MSRDailySkeletonInstance* result = 0;
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Skeleton, activity, actor, sample);
    result = new MSRDailySkeletonInstance(instanceInfo);
    return result;
}

DataInstance &MSRDailyActivity3D::getColorInstance(int activity, int actor, int sample)
{
    Q_UNUSED(activity);
    Q_UNUSED(actor);
    Q_UNUSED(sample);
}

}
