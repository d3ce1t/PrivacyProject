#ifndef DATA_INSTANCE_H
#define DATA_INSTANCE_H

#include "InstanceInfo.h"
#include "../types/StreamInstance.h"
#include <QReadWriteLock>

namespace dai {

class DataInstance : public StreamInstance
{
public:
    static float normalise(float value, float minValue, float maxValue, float newMin, float newMax);

    explicit DataInstance(const InstanceInfo& info);
    virtual ~DataInstance();
    const InstanceInfo& getMetadata() const;
    int getTotalFrames() const;

    // Overriden methods
    virtual bool is_open() const Q_DECL_OVERRIDE;
    bool hasNext() const Q_DECL_OVERRIDE;
    void open() Q_DECL_OVERRIDE;
    virtual void close() Q_DECL_OVERRIDE;
    void readNextFrame() Q_DECL_OVERRIDE;
    DataFrame& frame() Q_DECL_OVERRIDE;

protected:
    virtual void nextFrame(DataFrame& frame);
    virtual void restart();

    void initFrameBuffer(DataFrame* firstBuffer, DataFrame* secondBuffer);

    InstanceInfo m_info;
    int          m_frameIndex;
    int          m_nFrames;

private:
    void swapBuffer();

    QReadWriteLock    m_locker;
    DataFrame*        m_writeFrame;
    DataFrame*        m_readFrame;
};

}
#endif // DATA_INSTANCE_H
