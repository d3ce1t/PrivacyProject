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

class Dataset;

class DatasetMetadata
{
    friend class Dataset;

public:
    static shared_ptr<DatasetMetadata> load(QString xmlPath);
    static shared_ptr<DatasetMetadata> load_version1(QString xmlPath);
    static shared_ptr<DatasetMetadata> load_version2(QString xmlPath);
    static shared_ptr<DatasetMetadata> load_version3(QString xmlPath);

    virtual ~DatasetMetadata();
    const QString getName() const;
    const QString getDescription() const;
    const QString getPath() const;
    const shared_ptr<InstanceInfo> instance(int actor, int camera, int sample, const QList<QString>& labels);
    const shared_ptr<InstanceInfo> instance(int actor, int camera, int sample, const QList<QString>& labels, DataFrame::FrameType type);
    const QList<shared_ptr<InstanceInfo>> instance(int actor, int camera, int sample);
    const QList<shared_ptr<InstanceInfo>> instances(DataFrame::FrameType type, const QList<int> *actors = 0, const QList<int> *cameras = 0, const QList<int> *samples = 0, const QList<QList<QString> > *labels = 0) const;
    const Dataset& dataset() const;
    const DataFrame::SupportedFrames availableInstanceTypes() const;
    const QMap<int, QString>& actors() const;
    const QMap<int, QString>& cameras() const;
    const QMap<int, QString>& sampleTypes() const;
    const QMap<QString, QString>& labels() const;

private:
    void setDataset(Dataset* dataset);

    // Private Constructor
    explicit DatasetMetadata() {}
    void addInstanceInfo(shared_ptr<InstanceInfo> instance);

    QString                      m_name;
    QString                      m_description;
    QString                      m_path;
    QMap<int, QString>           m_actors;
    QMap<int, QString>           m_cameras;
    QMap<int, QString>           m_sampleTypes; // Compatibility with version 1 y 2
    QMap<QString, QString>       m_labels;
    Dataset*                     m_dataset;
    DataFrame::SupportedFrames   m_availableInstanceTypes;

    // List of instances
    QList<shared_ptr<InstanceInfo>> m_instances;
};

} // End Namespace

#endif // DATASETINFO_H
