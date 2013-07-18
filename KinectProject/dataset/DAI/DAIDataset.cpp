#include "DAIDataset.h"

namespace dai {

DAIDataset::DAIDataset()
    : Dataset("/home/jose/Dropbox/DataSet_Descriptor/DAI Test.xml")
{
}

shared_ptr<DataInstance> DAIDataset::getDepthInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Depth, activity, actor, sample);
    return shared_ptr<DataInstance>(new DAIDepthInstance(instanceInfo));
}

shared_ptr<DataInstance> DAIDataset::getSkeletonInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Skeleton, activity, actor, sample);
    return shared_ptr<DataInstance>(new DataInstance(instanceInfo));
}

shared_ptr<DataInstance> DAIDataset::getColorInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Color, activity, actor, sample);
    return shared_ptr<DataInstance>(new DAIColorInstance(instanceInfo));
}

shared_ptr<DataInstance> DAIDataset::getUserInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::User, activity, actor, sample);
    return shared_ptr<DataInstance>(new DAIUserInstance(instanceInfo));
}

} // End namespace
