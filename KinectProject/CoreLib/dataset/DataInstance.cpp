#include "DataInstance.h"

namespace dai {

DataInstance::DataInstance(const InstanceInfo &info)
    : StreamInstance(info.getType())
    , m_info(info)
{
    m_nFrames = 0;
}

const InstanceInfo& DataInstance::getMetadata() const
{
    return m_info;
}

unsigned int DataInstance::getTotalFrames() const
{
    return m_nFrames;
}

bool DataInstance::hasNext() const
{
    if (this->is_open() && (this->getFrameIndex() + 1 < m_nFrames))
        return true;

    return false;
}

} // End Namespace