#ifndef DATA_INSTANCE_H
#define DATA_INSTANCE_H

#include "InstanceInfo.h"
#include "DataFrame.h"

namespace dai {

class DataInstance
{
public:
    explicit DataInstance(const InstanceInfo& info);
    virtual void open() = 0;
    virtual void close() = 0;
    virtual int getTotalFrames() = 0;
    virtual bool hasNext() = 0;
    virtual DataFrame* nextFrame() = 0;

protected:
    InstanceInfo m_info;
};

}
#endif // DATA_INSTANCE_H
