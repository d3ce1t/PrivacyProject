#ifndef DATASET_H
#define DATASET_H

#include "DatasetMetadata.h"
#include <QString>
#include "DataInstance.h"


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
    virtual DataInstance* getDepthInstance(int activity, int actor, int sample) const = 0;
    virtual DataInstance* getSkeletonInstance(int activity, int actor, int sample) const = 0;
    virtual DataInstance* getColorInstance(int activity, int actor, int sample) const = 0;

    DataInstance* getSkeletonInstance(InstanceInfo info) const;
    DataInstance* getDepthInstance(InstanceInfo info) const;
    DataInstance* getColorInstance(InstanceInfo info) const;

protected:
    DatasetMetadata* m_metadata;
};

}

#endif // DATASETINFO_H
