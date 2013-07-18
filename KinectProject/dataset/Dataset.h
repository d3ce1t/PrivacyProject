#ifndef DATASET_H
#define DATASET_H

#include "DatasetMetadata.h"
#include <QString>
#include "DataInstance.h"
#include <memory>

using namespace std;

namespace dai {

class Dataset
{
public:

    enum DatasetType {
        Dataset_MSRDailyActivity3D = 0,
        Dataset_MSRAction3D,
        Dataset_DAI
    };

    explicit Dataset(QString path);
    virtual ~Dataset();
    const DatasetMetadata& getMetadata() const;

    virtual shared_ptr<DataInstance> getDepthInstance(int activity, int actor, int sample) const = 0;
    virtual shared_ptr<DataInstance> getSkeletonInstance(int activity, int actor, int sample) const = 0;
    virtual shared_ptr<DataInstance> getColorInstance(int activity, int actor, int sample) const = 0;
    virtual shared_ptr<DataInstance> getUserInstance(int activity, int actor, int sample) const = 0;

    shared_ptr<DataInstance> getDepthInstance(InstanceInfo info) const;
    shared_ptr<DataInstance> getSkeletonInstance(InstanceInfo info) const;
    shared_ptr<DataInstance> getColorInstance(InstanceInfo info) const;
    shared_ptr<DataInstance> getUserInstance(InstanceInfo info) const;

protected:
    DatasetMetadata* m_metadata;
};

}

#endif // DATASETINFO_H
