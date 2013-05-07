#include "DataInstance.h"

using namespace dai;

DataInstance::DataInstance(const InstanceInfo &info)
    : m_info(info)
{
    if (info.getType() == InstanceInfo::Depth) {
        this->m_type = StreamInstance::Depth;
    } else if (info.getType() == InstanceInfo::Skeleton) {
        this->m_type = StreamInstance::Skeleton;
    } if (info.getType() == InstanceInfo::Color) {
        this->m_type = StreamInstance::Color;
    }

    this->m_title = info.getFileName();
    m_playLoop = false;
}

DataInstance::~DataInstance()
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

//
// Static methods
//

float DataInstance::normalise(float value, float minValue, float maxValue, float newMin, float newMax)
{
    return ( (value - minValue) * (newMax - newMin) ) / (maxValue - minValue) + newMin;
}
