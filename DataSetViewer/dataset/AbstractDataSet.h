#ifndef ABSTRACTDATASET_H
#define ABSTRACTDATASET_H

#include "DataSetInfo.h"
#include "dataset/Sample.h"
#include <iostream>

using namespace std;

namespace DataSet {

    // Interface
    class AbstractDataSet
    {
    public:
        virtual bool open(string directory) = 0;
        virtual bool close() = 0;
        virtual bool hasNext() = 0;
        virtual Sample* next() = 0;
        //virtual DataSetInfo getInfo() = 0;
    };
}

#endif // ABSTRACTDATASET_H
