#ifndef STREAMINSTANCE_H
#define STREAMINSTANCE_H

#include "types/BaseInstance.h"
#include <QString>
#include "DataFrame.h"
#include <QReadWriteLock>
#include "exceptions/NotImplementedException.h"
#include "exceptions/NotOpenedInstanceException.h"
#include "exceptions/CannotOpenInstanceException.h"
#include <QDebug>

namespace dai {

template <class T>
class StreamInstance : public BaseInstance
{
public:
    StreamInstance(DataFrame::SupportedFrames supportedFrames);

#if (!defined _MSC_VER)
    StreamInstance(const StreamInstance& other) = delete;
#endif

    void open() override;
    void close() override;
    void restart() override;
    virtual bool hasNext() const override;
    void readNextFrame() override;
    QList< shared_ptr<DataFrame> > frames() override;
    void swapBuffer() override;
    unsigned int getFrameIndex() const;

protected:
    virtual bool openInstance() = 0;
    virtual void closeInstance() = 0;
    virtual void restartInstance() = 0;
    virtual void nextFrame(T& frame) = 0;
    void initFrameBuffer(shared_ptr<T> firstBuffer, shared_ptr<T> secondBuffer);

    QReadWriteLock  m_locker;
    shared_ptr<T>   m_readFrame;

private:

#if (defined _MSC_VER)
    StreamInstance(const StreamInstance&) {}
#endif

    unsigned int    m_frameIndex;    
    shared_ptr<T>   m_writeFrame;
};


template <class T>
StreamInstance<T>::StreamInstance(DataFrame::SupportedFrames supportedFrames)
    : BaseInstance(supportedFrames)
{
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
        if (!openInstance()) {
            throw CannotOpenInstanceException();
        }
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
QList< shared_ptr<DataFrame> > StreamInstance<T>::frames()
{
    QReadLocker locker(&m_locker);
    QList<shared_ptr<DataFrame>> result;
    result.append(static_pointer_cast<DataFrame>(m_readFrame));
    return result;
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
