#ifndef INSTANCE_INFO_H
#define INSTANCE_INFO_H

#include "DatasetEnums.h"
#include <QString>

namespace dai {

class InstanceInfo
{
public:
    InstanceInfo();
    explicit InstanceInfo(InstanceType type);
    InstanceInfo(const InstanceInfo& other);

    InstanceType getType() const;

    int getActivity() const;
    int getActor() const;
    int getSample() const;
    const QString& getFileName() const;
    const QString& getDatasetPath() const;
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
};

}

#endif // INSTANCE_INFO_H
