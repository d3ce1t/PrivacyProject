#ifndef DATASETMANAGER_H
#define DATASETMANAGER_H

enum SupportedDataSets {
    MSRDailyAction3D,
    MSRDailyActivity3D
};

class DataSetManager
{
public:
    static DataSet load(SupportedDataSets datasetType);
};

#endif // DATASETMANAGER_H
