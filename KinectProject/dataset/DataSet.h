#ifndef DATASET_H
#define DATASET_H

#include "DataSetInfo.h"
#include <QString>


class DataSet
{
public:
    DataSet();
    /*getNumberOfSamples();
    getNumberOfActors();
    getNumberOfActivities();*/
private:
    DataSetInfo m_info;

};

#endif // DATASETINFO_H
