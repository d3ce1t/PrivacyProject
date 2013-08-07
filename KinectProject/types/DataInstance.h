#ifndef DATA_INSTANCE_H
#define DATA_INSTANCE_H

#include "dataset/InstanceInfo.h"
#include "types/StreamInstance.h"
#include "exceptions/NotImplementedException.h"
#include "exceptions/NotOpenedInstanceException.h"

namespace dai {

template <class T>
class DataInstance : public StreamInstance<T>
{
public:
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
    virtual void nextFrame(T& frame);

    InstanceInfo m_info;
    unsigned int m_nFrames;
};

template <class T>
DataInstance<T>::DataInstance(const InstanceInfo &info)
    : m_info(info)
{
    this->m_type = info.getType();
    this->m_title = info.getFileName();
    m_nFrames = 0;
}

template <class T>
DataInstance<T>::~DataInstance()
{
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

template <class T>
bool DataInstance<T>::is_open() const
{
    throw NotImplementedException();
}

template <class T>
void DataInstance<T>::openInstance()
{
    throw NotImplementedException();
}

template <class T>
void DataInstance<T>::closeInstance()
{
    throw NotImplementedException();
}

template <class T>
void DataInstance<T>::restartInstance()
{
    throw NotImplementedException();
}

template <class T>
void DataInstance<T>::nextFrame(T& frame)
{
    Q_UNUSED(frame)
    throw NotImplementedException();
}

}
#endif // DATA_INSTANCE_H
