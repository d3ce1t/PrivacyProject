#include "DAI4REID.h"
#include "openni/OpenNIDevice.h"
#include "openni/OpenNIColorInstance.h"
#include "openni/OpenNIDepthInstance.h"
#include "openni/OpenNIUserTrackerInstance.h"

namespace dai {

DAI4REID::DAI4REID()
    : Dataset(":/data/DAI4REID.xml")
{
}

shared_ptr<StreamInstance> DAI4REID::instance(int actor, int camera, int sample, const QList<QString> &label, DataFrame::FrameType type) const
{
    const shared_ptr<InstanceInfo> instanceInfo = m_metadata->instance(actor, camera, sample, label, type);
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
