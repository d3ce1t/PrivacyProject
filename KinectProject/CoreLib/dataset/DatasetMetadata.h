#ifndef DATASETINFO_H
#define DATASETINFO_H

#include <QString>
#include <QHash>
#include <QMap>
#include <QList>
#include "InstanceInfo.h"
#include <memory>

using namespace std;

namespace dai {

typedef QList<InstanceInfo*> InstanceInfoList;
class Dataset;

class DatasetMetadata
{
    friend class Dataset;

public:
    static shared_ptr<DatasetMetadata> load(QString xmlPath);

    virtual ~DatasetMetadata();
    const QString getName() const;
    const QString getDescription() const;
    const QString getPath() const;
    int getNumberOfActivities() const;
    int getNumberOfActors() const;
    int getNumberOfSampleTypes() const;
    const InstanceInfoList* instances(DataFrame::FrameType type, const QList<int> *activities = 0, const QList<int> *actors = 0, const QList<int> *samples = 0) const;
    const InstanceInfo instance(DataFrame::FrameType type, int activity, int actor, int sample);
    const QString& getActivityName(int key) const;
    const QString& getActorName(int key) const;
    const QString& getSampleName(int key) const;
    const Dataset& dataset() const;
    const DataFrame::SupportedFrames availableInstanceTypes() const;

private:
    void setDataset(Dataset* dataset);

    // Private Constructor
    explicit DatasetMetadata();
    void addInstanceInfo(InstanceInfo *instance);

    QString                      m_name;
    QString                      m_description;
    QString                      m_path;
    int                          m_numberOfActivities;
    int                          m_numberOfActors;
    int                          m_numberOfSampleTypes;
    QHash<int, QString*>         m_activities;
    QHash<int, QString*>         m_actors;
    QHash<int, QString*>         m_sampleTypes;
    Dataset*                     m_dataset;
    DataFrame::SupportedFrames   m_availableInstanceTypes;

    /**
     * @brief m_instances
     *
     * Each instance type (depth, color, skeleton) has a hash of InstanceInfo* elements
     * indexed by activity keys. Each instanceInfo must match the given key.
     */
    QHash<DataFrame::FrameType, QHash<int, InstanceInfoList*>* > m_instances;
};

} // End Namespace

#endif // DATASETINFO_H
