#ifndef INSTANCE_INFO_H
#define INSTANCE_INFO_H

#include <QString>
#include "types/DataFrame.h"

namespace dai {

class DatasetMetadata;

class InstanceInfo
{
public:
    InstanceInfo(shared_ptr<DatasetMetadata> parent = nullptr);
    InstanceInfo(const InstanceInfo& other);
    InstanceInfo& operator=(const InstanceInfo& other);

    DataFrame::SupportedFrames getType() const;
    int getActivity() const;
    int getActor() const;
    int getSample() const;
    QString getFileName(DataFrame::FrameType frameType) const;
    const DatasetMetadata& parent() const;

    void addFileName(DataFrame::FrameType frameType, QString fileName);
    void addType(DataFrame::FrameType type);
    void setActivity(int activity);
    void setActor(int actor);
    void setSample(int sample);

private:
    DataFrame::SupportedFrames m_type;
    int m_activity;
    int m_actor;
    int m_sample;
    shared_ptr<DatasetMetadata> m_parent;
    QHash<DataFrame::FrameType, QString> m_files;
};

typedef QList<InstanceInfo*> InstanceInfoList;

}

#endif // INSTANCE_INFO_H
