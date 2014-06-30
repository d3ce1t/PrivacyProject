#ifndef DATA_INSTANCE_H
#define DATA_INSTANCE_H

#include "dataset/InstanceInfo.h"
#include "types/StreamInstance.h"

namespace dai {

template <class T>
class DataInstance : public StreamInstance<T>
{
public:
    explicit DataInstance(const InstanceInfo& info);

#if (!defined _MSC_VER)
    DataInstance(const DataInstance& other) = delete;
    virtual ~DataInstance() = default;
#else
    virtual ~DataInstance() {}
#endif

    const InstanceInfo& getMetadata() const;
    unsigned int getTotalFrames() const;
    bool hasNext() const override;

protected:
    InstanceInfo m_info;
    unsigned int m_nFrames;

private:

#if (defined _MSC_VER)
    DataInstance(const DataInstance&) {}
#endif
};

template <class T>
DataInstance<T>::DataInstance(const InstanceInfo &info)
    : StreamInstance(info.getType())
    , m_info(info)
{
    this->m_title = info.getFileName();
    m_nFrames = 0;
}

template <class T>
const InstanceInfo& DataInstance<T>::getMetadata() const
{
    return m_info;
}

template <class T>
unsigned int DataInstance<T>::getTotalFrames() const
{
    return m_nFrames;
}

template <class T>
bool DataInstance<T>::hasNext() const
{
    if (this->is_open() && (this->getFrameIndex() + 1 < m_nFrames))
        return true;

    return false;
}

}
#endif // DATA_INSTANCE_H
