#include "DataInstance.h"

namespace dai {

DataInstance::DataInstance(const InstanceInfo &info, DataFrame::SupportedFrames type)
    : StreamInstance(type)
    , m_info(info)
{
    m_nFrames = 0;
}

DataInstance::DataInstance(const InstanceInfo &info, DataFrame::SupportedFrames type, int width, int height)
    : StreamInstance(type, width, height)
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
    if (this->is_open() && (this->getFrameCounter() + 1 < m_nFrames))
        return true;

    return false;
}

} // End Namespace
