#include "CAVIAR4REID.h"
#include "CAVIAR4REIDInstance.h"

namespace dai {

CAVIAR4REID::CAVIAR4REID()
    : Dataset(":/data/CAVIAR4REID.xml")
{
}

shared_ptr<StreamInstance> CAVIAR4REID::instance(int actor, int camera, int sample, const QList<QString> &label, DataFrame::FrameType type) const
{
    const shared_ptr<InstanceInfo> instanceInfo = m_metadata->instance(actor, camera, sample, label, type);

    switch (type) {
    case DataFrame::Color:
        return make_shared<CAVIAR4REIDInstance>(*instanceInfo);
        break;
    default:
        return nullptr;
    }
}

}
