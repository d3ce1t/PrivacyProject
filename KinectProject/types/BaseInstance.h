#ifndef BASEINSTANCE_H
#define BASEINSTANCE_H

#include <QString>
#include "types/DataFrame.h"

namespace dai {

enum InstanceType {
    INSTANCE_DEPTH,
    INSTANCE_COLOR,
    INSTANCE_SKELETON,
    INSTANCE_USER,
    INSTANCE_UNINITIALISED
};

class BaseInstance
{
public:
    BaseInstance();
    virtual ~BaseInstance() = default;
    InstanceType getType() const;
    const QString& getTitle() const;

    virtual void open() = 0;
    virtual void close() = 0;
    virtual void restart() = 0;
    virtual bool is_open() const = 0;
    virtual bool hasNext() const = 0;
    virtual void swapBuffer() = 0;
    virtual void readNextFrame() = 0;
    virtual shared_ptr<DataFrame> frame() = 0;

protected:
    InstanceType    m_type;
    QString         m_title;
};

} // End Namespace

#endif // BASEINSTANCE_H
