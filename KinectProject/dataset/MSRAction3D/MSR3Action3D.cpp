#include "MSR3Action3D.h"


namespace dai {

MSR3Action3D::MSR3Action3D()
    : Dataset("/home/jose/Dropbox/DataSet_Descriptor/MSRAction3D.xml")
{
}

shared_ptr<DataInstance> MSR3Action3D::getDepthInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Depth, activity, actor, sample);
    return shared_ptr<DataInstance>(new MSRActionDepthInstance(instanceInfo));
}

shared_ptr<DataInstance> MSR3Action3D::getSkeletonInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Skeleton, activity, actor, sample);
    return shared_ptr<DataInstance>(new MSRActionSkeletonInstance(instanceInfo));
}

shared_ptr<DataInstance> MSR3Action3D::getColorInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Color, activity, actor, sample);
    return shared_ptr<DataInstance>(new DataInstance(instanceInfo));
}

shared_ptr<DataInstance> MSR3Action3D::getUserInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::User, activity, actor, sample);
    return shared_ptr<DataInstance>(new DataInstance(instanceInfo));
}

} // End namespace
