#ifndef DATASETENUMS_H
#define DATASETENUMS_H

namespace dai {

enum DatasetType {
    Dataset_MSRDailyAction3D,
    Dataset_MSRDailyActivity3D
};

enum InstanceType {
    Depth,
    Color,
    Skeleton,
    Uninitialised
};

}

#endif // DATASETENUMS_H
