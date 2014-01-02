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
    INSTANCE_USERTRACKER,
    INSTANCE_UNINITIALISED
};

class BaseInstance
{
public:
    BaseInstance();

#if (!defined _MSC_VER)
    BaseInstance(const BaseInstance& other) = delete;
    virtual ~BaseInstance() = default;
#else
    virtual ~BaseInstance() {}
#endif

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
    virtual QList<shared_ptr<DataFrame>> frames() = 0;

protected:
    InstanceType    m_type;
    QString         m_title;

private:
#if (defined _MSC_VER)
     BaseInstance(const BaseInstance&) {}
#endif

};

} // End Namespace

#endif // BASEINSTANCE_H
