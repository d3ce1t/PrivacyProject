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

    virtual bool is_open() const Q_DECL_OVERRIDE;
    bool hasNext() const Q_DECL_OVERRIDE;

protected:
    virtual void openInstance() Q_DECL_OVERRIDE;
    virtual void closeInstance() Q_DECL_OVERRIDE;
    virtual void restartInstance() Q_DECL_OVERRIDE;
    virtual void nextFrame(DataFrame& frame);

    InstanceInfo m_info;
    unsigned int m_nFrames;
};

}
#endif // DATA_INSTANCE_H
