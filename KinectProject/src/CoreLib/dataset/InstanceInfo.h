#ifndef INSTANCE_INFO_H
#define INSTANCE_INFO_H

#include <QString>
#include <QSet>
#include "types/DataFrame.h"

namespace dai {

class DatasetMetadata;

class InstanceInfo
{
    int m_actor;
    int m_sample;
    int m_camera;
    QSet<QString> m_labels;
    DataFrame::SupportedFrames m_type;
    shared_ptr<DatasetMetadata> m_parent;
    QHash<DataFrame::FrameType, QString> m_files;

public:
    // Constructors
    InstanceInfo(shared_ptr<DatasetMetadata> parent = nullptr);
    InstanceInfo(const InstanceInfo& other);

    // Operators
    bool operator==(const InstanceInfo& other) const;
    InstanceInfo& operator=(const InstanceInfo& other);


    // Getters
    int getActor() const;
    int getCamera() const;
    int getSample() const;
    bool hasLabels(const QList<QString>& labels) const;
    QList<QString> getLabels() const;
    DataFrame::SupportedFrames getType() const;
    QString getFileName(DataFrame::FrameType frameType) const;
    const DatasetMetadata& parent() const;

    // Setters
    void setActor(int actor);
    void setCamera(int camera);
    void setSample(int sample);
    void addLabel(const QString& label);
    void addFileName(DataFrame::FrameType frameType, QString fileName);
    void addType(DataFrame::FrameType type);
};

}

#endif // INSTANCE_INFO_H
