#include "MSRDailyActivity3D.h"
#include "MSRDailyDepthInstance.h"

namespace dai {

MSRDailyActivity3D::MSRDailyActivity3D()
    : Dataset("/home/jose/Dropbox/DataSet_Descriptor/MSRDailyActivity3D.xml")
{
}

shared_ptr<DataInstance> MSRDailyActivity3D::getDepthInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Depth, activity, actor, sample);
    return shared_ptr<DataInstance>(new MSRDailyDepthInstance(instanceInfo));
}

shared_ptr<DataInstance> MSRDailyActivity3D::getSkeletonInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Skeleton, activity, actor, sample);
    return shared_ptr<DataInstance>(new MSRDailySkeletonInstance(instanceInfo));
}

shared_ptr<DataInstance> MSRDailyActivity3D::getColorInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Color, activity, actor, sample);
    return shared_ptr<DataInstance>(new DataInstance(instanceInfo));
}

shared_ptr<DataInstance> MSRDailyActivity3D::getUserInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::User, activity, actor, sample);
    return shared_ptr<DataInstance>(new DataInstance(instanceInfo));
}

}
