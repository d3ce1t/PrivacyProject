#ifndef DATA_INSTANCE_H
#define DATA_INSTANCE_H

#include "InstanceInfo.h"
#include "types/DataFrame.h"

namespace dai {

class DataInstance
{
public:

    static float normalise(float value, float minValue, float maxValue, float newMin, float newMax);

    explicit DataInstance(const InstanceInfo& info);
    virtual ~DataInstance();
    const InstanceInfo& getMetadata() const;
    virtual int getTotalFrames() const = 0;
    virtual bool hasNext() const = 0;
    virtual void open() = 0;
    virtual void close() = 0;
    virtual const DataFrame& nextFrame() = 0;
    void setPlayLoop(bool value);

protected:
    InstanceInfo m_info;
    bool m_playLoop;
};

}
#endif // DATA_INSTANCE_H
