#ifndef DATA_INSTANCE_H
#define DATA_INSTANCE_H

#include "dataset/InstanceInfo.h"
#include "types/StreamInstance.h"

namespace dai {

class DataInstance : public StreamInstance
{
public:
    explicit DataInstance(const InstanceInfo& info);
    DataInstance(const DataInstance& other) = delete;
    const InstanceInfo& getMetadata() const;
    unsigned int getTotalFrames() const;
    bool hasNext() const override;

protected:
    InstanceInfo m_info;
    unsigned int m_nFrames;
}
;
} // End Namespace

#endif // DATA_INSTANCE_H
