#ifndef INSTANCE_INFO_H
#define INSTANCE_INFO_H

#include <QString>
#include "types/BaseInstance.h"
#include "types/DataFrame.h"

namespace dai {

class DatasetMetadata;

class InstanceInfo
{
public:
    InstanceInfo(shared_ptr<DatasetMetadata> parent = nullptr);
    explicit InstanceInfo(DataFrame::FrameType type, shared_ptr<DatasetMetadata> parent = nullptr);
    InstanceInfo(const InstanceInfo& other);
    virtual ~InstanceInfo();

    DataFrame::FrameType getType() const;
    int getActivity() const;
    int getActor() const;
    int getSample() const;
    QString getFileName() const;
    const DatasetMetadata& parent() const;
    void setType(DataFrame::FrameType type);
    void setActivity(int activity);
    void setActor(int actor);
    void setSample(int sample);
    void setFileName(QString file);

    InstanceInfo& operator=(const InstanceInfo& other);

private:
    DataFrame::FrameType m_type;
    int m_activity;
    int m_actor;
    int m_sample;
    QString m_file;
    shared_ptr<DatasetMetadata> m_parent;
};

}

#endif // INSTANCE_INFO_H
