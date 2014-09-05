#include "DAI4REID_Parsed.h"
#include "DAI4REID_ParsedInstance.h"

namespace dai {

DAI4REID_Parsed::DAI4REID_Parsed()
    : Dataset(":/data/DAI4REID_parsed.xml")
{
}

shared_ptr<StreamInstance> DAI4REID_Parsed::instance(int actor, int camera, int sample, const QList<QString> &label, DataFrame::FrameType type) const
{
    const shared_ptr<InstanceInfo> instanceInfo = m_metadata->instance(actor, camera, sample, label, type);

    if (type == DataFrame::Color || type == DataFrame::Depth || type == DataFrame::Mask || type == DataFrame::Skeleton)
    {
         return make_shared<DAI4REID_ParsedInstance>(*instanceInfo);
    }

    return nullptr;
}

}
