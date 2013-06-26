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
    }

    m_title = info.getFileName();
    m_nFrames = 0;
    m_frameIndex = 0;
    m_writeFrame = NULL;
    m_readFrame = NULL;
}

DataInstance::~DataInstance()
{
    m_nFrames = 0;
    m_frameIndex = 0;
    m_writeFrame = NULL;
    m_readFrame = NULL;
}

void DataInstance::initFrameBuffer(DataFrame* firstBuffer, DataFrame* secondBuffer)
{
   m_writeFrame = firstBuffer;
   m_readFrame = secondBuffer;
}

const InstanceInfo& DataInstance::getMetadata() const
{
    return m_info;
}

int DataInstance::getTotalFrames() const
{
    return m_nFrames;
}

bool DataInstance::hasNext() const
{
    if (this->is_open() && (m_frameIndex < m_nFrames))
        return true;

    return false;
}

bool DataInstance::is_open() const
{
    throw NotImplementedException();
}

void DataInstance::open()
{
    throw NotImplementedException();
}

void DataInstance::close()
{
    throw NotImplementedException();
}

void DataInstance::readNextFrame()
{
    if (!is_open()) {
        throw NotOpenedInstanceException();
    }

    if (m_frameIndex < m_nFrames)
    {
        m_writeFrame->setIndex(m_frameIndex);
        nextFrame(*m_writeFrame);
        m_frameIndex++;
    }
    else {
        close();
    }

    swapBuffer();
}

DataFrame& DataInstance::frame()
{
    QReadLocker locker(&m_locker);
    return *m_readFrame;
}

void DataInstance::nextFrame(DataFrame& frame)
{
    Q_UNUSED(frame)
    throw NotImplementedException();
}

void DataInstance::restart()
{
    throw NotImplementedException();
}

void DataInstance::swapBuffer()
{
    QWriteLocker locker(&m_locker);
    DataFrame* tmpPtr = m_readFrame;
    m_readFrame = m_writeFrame;
    m_writeFrame = tmpPtr;
}

//
// Static methods
//

float DataInstance::normalise(float value, float minValue, float maxValue, float newMin, float newMax)
{
    return ( (value - minValue) * (newMax - newMin) ) / (maxValue - minValue) + newMin;
}
