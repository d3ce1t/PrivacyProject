#ifndef DATASETINFO_H
#define DATASETINFO_H

#include <QString>

class DataSetInfo
{
public:
    DataSetInfo();
    QString getDescription();
    QString getPath();
    int getNumberOfActivities();
    int getNumberOfActors();
    int getNumberOfSampleTypes();

private:
    QString m_description;
    QString m_path;
    int m_numberOfActivities;
    int m_numberOfActors;
    int m_numberOfSampleTypes;


};

#endif // DATASETINFO_H
