#include "DAIDataset.h"

namespace dai {

DAIDataset::DAIDataset()
    : Dataset("/home/jose/Dropbox/DataSet_Descriptor/DAI Test.xml")
{
}

DAIDepthInstance* DAIDataset::getDepthInstance(int activity, int actor, int sample)
{
    DAIDepthInstance* result = 0;
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Depth, activity, actor, sample);
    result = new DAIDepthInstance(instanceInfo);
    return result;
}

DataInstance* DAIDataset::getSkeletonInstance(int activity, int actor, int sample)
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Depth, activity, actor, sample);
    return new DataInstance(instanceInfo);
}

DataInstance* DAIDataset::getColorInstance(int activity, int actor, int sample)
{
    const InstanceInfo instanceInfo = m_metadata->instance(InstanceInfo::Depth, activity, actor, sample);
    return new DataInstance(instanceInfo);
}

} // End namespace
