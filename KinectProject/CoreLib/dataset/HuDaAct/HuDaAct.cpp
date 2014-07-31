#include "HuDaAct.h"
#include "openni/OpenNIDevice.h"
#include "openni/OpenNIColorInstance.h"
#include "openni/OpenNIDepthInstance.h"
#include "openni/OpenNIUserTrackerInstance.h"

namespace dai {

HuDaAct::HuDaAct()
    : Dataset(":/data/HuDaAct.xml")
{
}

shared_ptr<StreamInstance> HuDaAct::instance(int activity, int actor, int sample, DataFrame::FrameType type) const
{
    const InstanceInfo* instanceInfo = m_metadata->instance(type, activity, actor, sample);
    QString datasetPath = instanceInfo->parent().getPath();
    QString instancePath = datasetPath + "/" + instanceInfo->getFileName(type);

    OpenNIDevice* oni_device = OpenNIDevice::create(instancePath);

    switch (type) {
    case DataFrame::Color:
        return make_shared<OpenNIColorInstance>(oni_device);
        break;
    case DataFrame::Depth:
        return make_shared<OpenNIDepthInstance>(oni_device);
        break;
    case DataFrame::Mask:
        return make_shared<OpenNIUserTrackerInstance>(oni_device);
        break;
    case DataFrame::Skeleton:
        return make_shared<OpenNIUserTrackerInstance>(oni_device);
        break;
    default:
        return nullptr;
    }
}

} // End Namespace
