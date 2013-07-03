#include "StreamInstance.h"
#include <QDebug>
#include "exceptions/NotImplementedException.h"
#include "exceptions/NotOpenedInstanceException.h"

namespace dai {

StreamInstance::StreamInstance()
{
    m_type = StreamInstance::Uninitialised;
    m_writeFrame = NULL;
    m_readFrame = NULL;
    m_frameIndex = 0;
}

StreamInstance::~StreamInstance()
{
    m_type = StreamInstance::Uninitialised;
    m_writeFrame = NULL;
    m_readFrame = NULL;
    m_frameIndex = 0;
    qDebug() << "StreamInstance::~StreamInstance()";
}

StreamInstance::StreamType StreamInstance::getType() const
{
    return m_type;
}

const QString& StreamInstance::getTitle() const
{
    return m_title;
}

unsigned int StreamInstance::getFrameIndex() const
{
    return m_frameIndex - 1;
}

void StreamInstance::open()
{
    if (!is_open()) {
        m_frameIndex = 0;
        openInstance();
    }
}

void StreamInstance::close()
{
    if (is_open()) {
        closeInstance();
    }
}

void StreamInstance::restart()
{
    if (is_open()) {
        restartInstance();
        m_frameIndex = 0;
    }
}

void StreamInstance::readNextFrame()
{
    if (!is_open()) {
        throw NotOpenedInstanceException();
    }

    if (hasNext()) {
        m_writeFrame->setIndex(m_frameIndex);
        nextFrame(*m_writeFrame);
        m_frameIndex++;
        swapBuffer();
    }
    else {
        closeInstance();
    }
}

bool StreamInstance::hasNext() const
{
    return true;
}

DataFrame& StreamInstance::frame()
{
    QReadLocker locker(&m_locker);
    return *m_readFrame;
}

void StreamInstance::swapBuffer()
{
    QWriteLocker locker(&m_locker);
    DataFrame* tmpPtr = m_readFrame;
    m_readFrame = m_writeFrame;
    m_writeFrame = tmpPtr;
}

void StreamInstance::initFrameBuffer(DataFrame* firstBuffer, DataFrame* secondBuffer)
{
   m_writeFrame = firstBuffer;
   m_readFrame = secondBuffer;
}

} // End namespace
