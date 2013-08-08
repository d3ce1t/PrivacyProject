#ifndef DATASET_H
#define DATASET_H

#include "DatasetMetadata.h"
#include <QString>
#include <memory>
#include "types/DataInstance.h"
#include "types/BaseInstance.h"
#include "types/DepthFrame.h"
#include "types/ColorFrame.h"
#include "types/SkeletonFrame.h"
#include "types/UserFrame.h"

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
    virtual ~Dataset() = default;
    const DatasetMetadata& getMetadata() const;

    shared_ptr<BaseInstance> getInstance(int activity, int actor, int sample, InstanceType type) const;
    shared_ptr<BaseInstance> getInstance(const InstanceInfo &info) const;

protected:
    virtual shared_ptr<BaseInstance> instance(int activity, int actor, int sample, InstanceType type) const = 0;

    shared_ptr<DatasetMetadata> m_metadata;
};

}

#endif // DATASETINFO_H
