#include "DataInstance.h"

using namespace dai;

DataInstance::DataInstance(const InstanceInfo &info)
    : m_info(info)
{
    m_playLoop = false;
}

const InstanceInfo& DataInstance::getMetadata() const
{
    return m_info;
}

void DataInstance::setPlayLoop(bool value)
{
    m_playLoop = value;
}
