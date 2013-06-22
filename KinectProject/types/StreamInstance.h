#ifndef STREAMINSTANCE_H
#define STREAMINSTANCE_H

#include <QString>
#include "DataFrame.h"

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
    virtual bool hasNext() const = 0;
    virtual bool is_open() const = 0;
    virtual void open() = 0;
    virtual void close() = 0;
    virtual void restart() = 0;
    virtual void readNextFrame() = 0;
    virtual DataFrame& frame() = 0;
    StreamType getType() const;
    const QString& getTitle() const;

protected:
    StreamType m_type;
    QString m_title;
};

} // End namespace

#endif // STREAMINSTANCE_H
