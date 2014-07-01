#ifndef STREAMINSTANCE_H
#define STREAMINSTANCE_H

#include <QString>
#include "types/DataFrame.h"
#include "exceptions/NotOpenedInstanceException.h"
#include "exceptions/CannotOpenInstanceException.h"
#include <QReadWriteLock>

namespace dai {

class StreamInstance
{
public:
    StreamInstance(DataFrame::SupportedFrames supportedFrames);

#if (!defined _MSC_VER)
    StreamInstance(const StreamInstance& other) = delete;
#endif

    virtual void open();
    virtual void close();
    virtual void restart();
    virtual bool is_open() const = 0;
    virtual bool hasNext() const;
    void readNextFrames();

    virtual QList< shared_ptr<DataFrame> > frames();
    unsigned int getFrameIndex() const;
    DataFrame::SupportedFrames getSupportedFrames() const;

protected:
    virtual bool openInstance() = 0;
    virtual void closeInstance() = 0;
    virtual void restartInstance() = 0;
    virtual QList< shared_ptr<DataFrame>> nextFrames() = 0;

private:

#if (defined _MSC_VER)
    StreamInstance(const StreamInstance&) {}
#endif

    unsigned int                  m_frameIndex;
    DataFrame::SupportedFrames    m_supportedFrames;
    QList< shared_ptr<DataFrame>> m_readFrames;
    QReadWriteLock                m_locker;
};

} // End namespace

#endif // STREAMINSTANCE_H
