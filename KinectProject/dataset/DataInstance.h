#ifndef DATA_INSTANCE_H
#define DATA_INSTANCE_H

#include "InstanceInfo.h"
#include "types/StreamInstance.h"

namespace dai {

class DataInstance : public StreamInstance
{
public:

    static float normalise(float value, float minValue, float maxValue, float newMin, float newMax);

    explicit DataInstance(const InstanceInfo& info);
    virtual ~DataInstance();
    const InstanceInfo& getMetadata() const;
    virtual int getTotalFrames() const = 0;
    void setPlayLoop(bool value);

protected:
    InstanceInfo m_info;
    bool m_playLoop;
};

}
#endif // DATA_INSTANCE_H
