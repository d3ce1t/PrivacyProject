#include "HuDaAct.h"
#include "openni/OpenNIColorInstance.h"
#include "openni/OpenNIDepthInstance.h"

namespace dai {

HuDaAct::HuDaAct()
    : Dataset(":/data/HuDaAct.xml")
{
}

shared_ptr<StreamInstance> HuDaAct::instance(int activity, int actor, int sample, DataFrame::FrameType type) const
{
    const InstanceInfo* instanceInfo = m_metadata->instance(type, activity, actor, sample);

    switch (type) {
    case DataFrame::Color:
        //return shared_ptr<StreamInstance>(new OpenNIColorInstance)
        break;
    case DataFrame::Depth:
        //return shared_ptr<StreamInstance>(new MSRActionDepthInstance(*instanceInfo));
        break;
    case DataFrame::Mask:
        break;
    case DataFrame::Skeleton:
        //return shared_ptr<StreamInstance>(new MSRActionSkeletonInstance(*instanceInfo));
        break;
    default:
        return nullptr;
    }
}

} // End Namespace
