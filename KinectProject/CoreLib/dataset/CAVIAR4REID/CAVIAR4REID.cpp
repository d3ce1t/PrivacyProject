#include "CAVIAR4REID.h"
#include "CAVIAR4REIDInstance.h"

namespace dai {

CAVIAR4REID::CAVIAR4REID()
    : Dataset(":/data/MSRDailyActivity3D.xml")
{
}

shared_ptr<StreamInstance> CAVIAR4REID::instance(int activity, int actor, int sample, DataFrame::FrameType type) const
{
    const InstanceInfo* instanceInfo = m_metadata->instance(type, activity, actor, sample);

    switch (type) {
    case DataFrame::Color:
        return make_shared<CAVIAR4REIDInstance>(*instanceInfo);
        break;
    default:
        return nullptr;
    }
}

}
