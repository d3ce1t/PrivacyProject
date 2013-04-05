#ifndef DATASET_H
#define DATASET_H

#include "DatasetMetadata.h"
#include <QString>
#include "DataInstance.h"


namespace dai {


class Dataset
{
public:
    explicit Dataset(QString path);
    const DatasetMetadata& getMetadata() const;

    virtual DataInstance& getDepthInstance(int activity, int actor, int sample) = 0;
    virtual DataInstance& getColorInstance(int activity, int actor, int sample) = 0;

protected:
    DatasetMetadata* m_metadata;
};

}

#endif // DATASETINFO_H
