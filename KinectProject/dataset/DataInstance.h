#ifndef DATA_INSTANCE_H
#define DATA_INSTANCE_H

#include "InstanceInfo.h"
#include "types/DataFrame.h"

namespace dai {

class DataInstance
{
public:

    static float normalise(float value, float minValue, float maxValue, float newMax, float newMin);

    explicit DataInstance(const InstanceInfo& info);
    const InstanceInfo& getMetadata() const;
    virtual void open() = 0;
    virtual void close() = 0;
    virtual int getTotalFrames() = 0;
    /*virtual int getResolutionX() = 0;
    virtual int getResolutionY() = 0;*/
    virtual bool hasNext() = 0;
    virtual const DataFrame& nextFrame() = 0;
    void setPlayLoop(bool value);

protected:
    InstanceInfo m_info;
    bool m_playLoop;
};

}
#endif // DATA_INSTANCE_H
