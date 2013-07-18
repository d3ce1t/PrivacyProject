#ifndef DATA_INSTANCE_H
#define DATA_INSTANCE_H

#include "InstanceInfo.h"
#include "../types/StreamInstance.h"


namespace dai {

class DataInstance : public StreamInstance
{
public:
    static float normalise(float value, float minValue, float maxValue, float newMin, float newMax);

    explicit DataInstance(const InstanceInfo& info);
    virtual ~DataInstance();
    const InstanceInfo& getMetadata() const;
    unsigned int getTotalFrames() const;

    virtual bool is_open() const override;
    bool hasNext() const override;

protected:
    virtual void openInstance() override;
    virtual void closeInstance() override;
    virtual void restartInstance() override;
    virtual void nextFrame(DataFrame& frame);

    InstanceInfo m_info;
    unsigned int m_nFrames;
};

}
#endif // DATA_INSTANCE_H
