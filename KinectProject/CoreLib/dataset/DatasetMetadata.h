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
    QString                      m_name;
    QString                      m_description;
    QString                      m_path;
    QMap<int, QString>           m_actors;
    QMap<int, QString>           m_cameras;
    QMap<QString, QString>       m_labels;
    Dataset*                     m_dataset;
    DataFrame::SupportedFrames   m_availableInstanceTypes;

    // List of instances
    QList<shared_ptr<InstanceInfo>> m_instances;

    friend class Dataset;

public:

    static const QList<QList<QString>> ANY_LABEL;
    static const QList<int> ALL_ACTORS;

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
    //const QList<shared_ptr<InstanceInfo>> instance(int actor, int camera, int sample);
    const QList<shared_ptr<InstanceInfo>> instances(DataFrame::FrameType type, const QList<int> &actors = QList<int>(), const QList<int> &cameras = QList<int>(), const QList<QList<QString> > &labels = QList<QList<QString>>()) const;
    const Dataset& dataset() const;
    const DataFrame::SupportedFrames availableInstanceTypes() const;
    const QMap<int, QString>& actors() const;
    const QMap<int, QString>& cameras() const;
    const QMap<QString, QString>& labels() const;

private:
    void setDataset(Dataset* dataset);

    // Private Constructor
    explicit DatasetMetadata() {}
    void addInstanceInfo(shared_ptr<InstanceInfo> instance);
};

} // End Namespace

#endif // DATASETINFO_H
