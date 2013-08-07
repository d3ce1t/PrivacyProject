#include "DAIDataset.h"
#include "DAIDepthInstance.h"
#include "DAIColorInstance.h"
#include "DAIUserInstance.h"
#include "DAISkeletonInstance.h"

namespace dai {

DAIDataset::DAIDataset()
    : Dataset("/home/jose/Dropbox/DataSet_Descriptor/DAI Test.xml")
{
}

shared_ptr<BaseInstance> DAIDataset::getInstance(int activity, int actor, int sample, InstanceType type) const
{
    const InstanceInfo instanceInfo = m_metadata->instance(type, activity, actor, sample);

    switch (type) {
    case INSTANCE_DEPTH:
        return shared_ptr<BaseInstance>(new DAIDepthInstance(instanceInfo));
        break;
    case INSTANCE_SKELETON:
        return shared_ptr<BaseInstance>(new DAISkeletonInstance(instanceInfo));
        break;
    case INSTANCE_COLOR:
        return shared_ptr<BaseInstance>(new DAIColorInstance(instanceInfo));
        break;
    case INSTANCE_USER:
        return shared_ptr<BaseInstance>(new DAIUserInstance(instanceInfo));
        break;
    default:
        return nullptr;
    }
}

} // End namespace
