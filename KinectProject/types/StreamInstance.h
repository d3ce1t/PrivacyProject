#ifndef STREAMINSTANCE_H
#define STREAMINSTANCE_H

#include "types/BaseInstance.h"
#include <QString>
#include "DataFrame.h"
#include <QReadWriteLock>
#include "exceptions/NotImplementedException.h"
#include "exceptions/NotOpenedInstanceException.h"
#include <QDebug>

namespace dai {

template <class T>
class StreamInstance : public BaseInstance
{
public:
    StreamInstance();
    virtual ~StreamInstance() = default;
    void open() override;
    void close() override;
    void restart() override;
    virtual bool hasNext() const override;
    void readNextFrame() override;
    shared_ptr<DataFrame> frame() override;
    void swapBuffer() override;
    unsigned int getFrameIndex() const;

protected:
    virtual void openInstance() = 0;
    virtual void closeInstance() = 0;
    virtual void restartInstance() = 0;
    virtual void nextFrame(T& frame) = 0;
    void initFrameBuffer(shared_ptr<T> firstBuffer, shared_ptr<T> secondBuffer);

private:
    unsigned int    m_frameIndex;
    QReadWriteLock  m_locker;
    shared_ptr<T>   m_writeFrame;
    shared_ptr<T>   m_readFrame;
};


template <class T>
StreamInstance<T>::StreamInstance()
{
    m_type = INSTANCE_UNINITIALISED;
    m_writeFrame = nullptr;
    m_readFrame = nullptr;
    m_frameIndex = 0;
}

template <class T>
unsigned int StreamInstance<T>::getFrameIndex() const
{
    return m_frameIndex - 1;
}

template <class T>
void StreamInstance<T>::open()
{
    if (!is_open()) {
        m_frameIndex = 0;
        openInstance();
    }
}

template <class T>
void StreamInstance<T>::close()
{
    if (is_open()) {
        closeInstance();
    }
}

template <class T>
void StreamInstance<T>::restart()
{
    if (is_open()) {
        restartInstance();
        m_frameIndex = 0;
    }
}

template <class T>
void StreamInstance<T>::readNextFrame()
{
    if (!is_open()) {
        throw NotOpenedInstanceException();
    }

    if (hasNext()) {
        m_writeFrame->setIndex(m_frameIndex);
        nextFrame(*m_writeFrame);
        m_frameIndex++;
    }
    else {
        closeInstance();
    }
}

template <class T>
bool StreamInstance<T>::hasNext() const
{
    return true;
}

template <class T>
shared_ptr<DataFrame> StreamInstance<T>::frame()
{
    QReadLocker locker(&m_locker);
    return static_pointer_cast<DataFrame>(m_readFrame);
}

template <class T>
void StreamInstance<T>::swapBuffer()
{
    QWriteLocker locker(&m_locker);
    shared_ptr<T> tmpPtr = m_readFrame;
    m_readFrame = m_writeFrame;
    m_writeFrame = tmpPtr;
}

template <class T>
void StreamInstance<T>::initFrameBuffer(shared_ptr<T> firstBuffer, shared_ptr<T> secondBuffer)
{
   m_writeFrame = firstBuffer;
   m_readFrame = secondBuffer;
}

} // End namespace

#endif // STREAMINSTANCE_H
