#ifndef INSTANCE_INFO_H
#define INSTANCE_INFO_H

#include <QString>

namespace dai {

class DatasetMetadata;

class InstanceInfo
{
public:

    enum InstanceType {
        Depth,
        Color,
        Skeleton,
        Uninitialised
    };

    InstanceInfo(DatasetMetadata* parent = NULL);
    explicit InstanceInfo(InstanceType type, DatasetMetadata* parent = NULL);
    InstanceInfo(const InstanceInfo& other);
    virtual ~InstanceInfo();

    InstanceType getType() const;

    int getActivity() const;
    int getActor() const;
    int getSample() const;
    QString getFileName() const;
    QString getDatasetPath() const;
    const DatasetMetadata* parent() const;
    void setType(InstanceType type);
    void setActivity(int activity);
    void setActor(int actor);
    void setSample(int sample);
    void setFileName(QString file);
    void setDatasetPath(QString path);

    InstanceInfo& operator=(const InstanceInfo& other);

private:
    InstanceType m_type;
    int m_activity;
    int m_actor;
    int m_sample;
    QString m_file;
    QString m_path;
    DatasetMetadata* m_parent;
};

}

#endif // INSTANCE_INFO_H
