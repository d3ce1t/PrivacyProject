#ifndef STREAMINSTANCE_H
#define STREAMINSTANCE_H

#include <QString>
#include "DataFrame.h"
#include <QReadWriteLock>

namespace dai {

class StreamInstance
{
public:
    enum StreamType {
        Depth,
        Color,
        Skeleton,
        Uninitialised
    };

    StreamInstance();
    virtual ~StreamInstance();
    void open();
    void close();
    void restart();
    virtual bool is_open() const = 0;
    virtual bool hasNext() const;

    void readNextFrame();
    DataFrame& frame();
    StreamType getType() const;
    const QString& getTitle() const;
    unsigned int getFrameIndex() const;

protected:
    virtual void openInstance() = 0;
    virtual void closeInstance() = 0;
    virtual void restartInstance() = 0;
    virtual void nextFrame(DataFrame& frame) = 0;


    void initFrameBuffer(DataFrame* firstBuffer, DataFrame* secondBuffer);

    StreamType      m_type;
    QString         m_title;

private:
    void swapBuffer();

    unsigned int    m_frameIndex;
    QReadWriteLock  m_locker;
    DataFrame*      m_writeFrame;
    DataFrame*      m_readFrame;
};

} // End namespace

#endif // STREAMINSTANCE_H
