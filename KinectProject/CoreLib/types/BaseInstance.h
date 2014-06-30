#ifndef BASEINSTANCE_H
#define BASEINSTANCE_H

#include <QString>
#include "types/DataFrame.h"

namespace dai {

class BaseInstance
{
public:
    BaseInstance(DataFrame::SupportedFrames supportedFrames);

#if (!defined _MSC_VER)
    BaseInstance(const BaseInstance& other) = delete;
    virtual ~BaseInstance() = default;
#else
    virtual ~BaseInstance() {}
#endif

    const QString& getTitle() const;

    virtual void open() = 0;
    virtual void close() = 0;
    virtual void restart() = 0;
    virtual bool is_open() const = 0;
    virtual bool hasNext() const = 0;
    virtual void swapBuffer() = 0;
    virtual void readNextFrame() = 0;
    virtual QList<shared_ptr<DataFrame>> frames() = 0;
    DataFrame::SupportedFrames getSupportedFrames() const;

protected:
    QString         m_title;
    DataFrame::SupportedFrames m_supportedFrames;

private:
#if (defined _MSC_VER)
     BaseInstance(const BaseInstance&) {}
#endif


};

} // End Namespace

#endif // BASEINSTANCE_H
