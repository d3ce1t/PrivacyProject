#ifndef DATASET_H
#define DATASET_H

#include "DatasetMetadata.h"
#include <QString>
#include <memory>
#include "DataInstance.h"

using namespace std;

namespace dai {

class Dataset
{
public:

    enum DatasetType {
        Dataset_MSRDailyActivity3D = 0,
        Dataset_MSRAction3D,
        Dataset_HuDaAct
    };

    explicit Dataset(const QString& xmlDescriptor);
    virtual ~Dataset() = default;
    void open(const QString& xmlDescriptor);
    void setPath(const QString& path);
    const DatasetMetadata& getMetadata() const;
    shared_ptr<StreamInstance> getInstance(int actor, int camera, int sample, const QList<QString>& labels, DataFrame::FrameType type) const;
    shared_ptr<StreamInstance> getInstance(const InstanceInfo &info, DataFrame::FrameType type) const;

protected:
    virtual shared_ptr<StreamInstance> instance(int actor, int camera, int sample, const QList<QString>& labels, DataFrame::FrameType type) const = 0;

    shared_ptr<DatasetMetadata> m_metadata;
};

}

#endif // DATASETINFO_H
