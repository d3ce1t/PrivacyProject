#include "MSR3Action3D.h"


namespace dai {

MSR3Action3D::MSR3Action3D()
    : Dataset("/home/jose/Dropbox/DataSet_Descriptor/MSRAction3D.xml")
{
}

MSRActionDepthInstance *MSR3Action3D::getDepthInstance(int activity, int actor, int sample)
{
    MSRActionDepthInstance* result = 0;
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Depth, activity, actor, sample);
    result = new MSRActionDepthInstance(instanceInfo);
    return result;
}

MSRActionSkeletonInstance* MSR3Action3D::getSkeletonInstance(int activity, int actor, int sample)
{
    MSRActionSkeletonInstance* result = 0;
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Skeleton, activity, actor, sample);
    result = new MSRActionSkeletonInstance(instanceInfo);
    return result;
}

DataInstance* MSR3Action3D::getColorInstance(int activity, int actor, int sample)
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Skeleton, activity, actor, sample);
    return new DataInstance(instanceInfo);
}

} // End namespace
