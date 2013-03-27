#include "datasetmanager.h"

 DataSet DataSetManager::load(SupportedDataSets datasetType)
 {
     if (datasetType == MSRDailyActivity3D) {

     } else if (datasetType == MSRDailyAction3D) {

     } else {
         throw 0; // Not supported data set
     }
 }
