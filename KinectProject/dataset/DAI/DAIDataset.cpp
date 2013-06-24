#include "DAIDataset.h"

namespace dai {

DAIDataset::DAIDataset()
    : Dataset("/home/jose/Dropbox/DataSet_Descriptor/DAI Test.xml")
{
}

DAIDepthInstance* DAIDataset::getDepthInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Depth, activity, actor, sample);
    return new DAIDepthInstance(instanceInfo);
}

DataInstance* DAIDataset::getSkeletonInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Depth, activity, actor, sample);
    return new DataInstance(instanceInfo);
}

DAIColorInstance *DAIDataset::getColorInstance(int activity, int actor, int sample) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Color, activity, actor, sample);
    return new DAIColorInstance(instanceInfo);
}

} // End namespace
