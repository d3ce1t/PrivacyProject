#include "DataInstance.h"
#include "exceptions/NotImplementedException.h"
#include "exceptions/NotOpenedInstanceException.h"

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
    } else if (info.getType() == InstanceInfo::User) {
        this->m_type = StreamInstance::User;
    }

    m_title = info.getFileName();
    m_nFrames = 0;
}

DataInstance::~DataInstance()
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
    if (this->is_open() && (getFrameIndex() + 1 < m_nFrames))
        return true;

    return false;
}

bool DataInstance::is_open() const
{
    throw NotImplementedException();
}

void DataInstance::openInstance()
{
    throw NotImplementedException();
}

void DataInstance::closeInstance()
{
    throw NotImplementedException();
}

void DataInstance::restartInstance()
{
    throw NotImplementedException();
}

void DataInstance::nextFrame(DataFrame& frame)
{
    Q_UNUSED(frame)
    throw NotImplementedException();
}

//
// Static methods
//

float DataInstance::normalise(float value, float minValue, float maxValue, float newMin, float newMax)
{
    return ( (value - minValue) * (newMax - newMin) ) / (maxValue - minValue) + newMin;
}
