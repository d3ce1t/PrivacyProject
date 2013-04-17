#ifndef DATASETINFO_H
#define DATASETINFO_H

#include <QString>
#include <QHash>
#include <QList>
#include "InstanceInfo.h"


namespace dai {

typedef QList<InstanceInfo*> InstanceInfoList;


class DatasetMetadata
{

public:
    static DatasetMetadata* load(QString xmlPath);

    virtual ~DatasetMetadata();
    const QString getName() const;
    const QString getDescription() const;
    const QString getPath() const;
    int getNumberOfActivities() const;
    int getNumberOfActors() const;
    int getNumberOfSampleTypes() const;
    const InstanceInfoList* instances(InstanceInfo::InstanceType type, const QList<int> *activities = 0, const QList<int> *actors = 0, const QList<int> *samples = 0) const;
    const InstanceInfo instance(InstanceInfo::InstanceType type, int activity, int actor, int sample);
    const QString& getActivityName(int key) const;
    const QString& getActorName(int key) const;
    const QString& getSampleName(int key) const;

private:
    // Private Constructor
    explicit DatasetMetadata();
    void addInstanceInfo(InstanceInfo *instance);

    QString m_name;
    QString m_description;
    QString m_path;
    int m_numberOfActivities;
    int m_numberOfActors;
    int m_numberOfSampleTypes;
    QHash<int, QString*> m_activities;
    QHash<int, QString*> m_actors;
    QHash<int, QString*> m_sampleTypes;

    /**
     * @brief m_instances
     *
     * Each instance type (depth, color, skeleton) has a hash of InstanceInfo* elements
     * indexed by activity keys. Each instanceInfo must match the given key.
     */
    QHash<InstanceInfo::InstanceType, QHash<int, InstanceInfoList*>* > m_instances;
};

}

#endif // DATASETINFO_H
