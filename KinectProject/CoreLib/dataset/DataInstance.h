#ifndef DATA_INSTANCE_H
#define DATA_INSTANCE_H

#include "dataset/InstanceInfo.h"
#include "types/StreamInstance.h"

namespace dai {

class DataInstance : public StreamInstance
{
public:
    explicit DataInstance(const InstanceInfo& info);

#if (!defined _MSC_VER)
    DataInstance(const DataInstance& other) = delete;
#endif

    const InstanceInfo& getMetadata() const;
    unsigned int getTotalFrames() const;
    bool hasNext() const override;

protected:
    InstanceInfo m_info;
    unsigned int m_nFrames;

private:
#if (defined _MSC_VER)
    DataInstance(const DataInstance&)
        :StreamInstance(DataFrame::Unknown){}
#endif
}
;
} // End Namespace

#endif // DATA_INSTANCE_H
