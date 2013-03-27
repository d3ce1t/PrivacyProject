#ifndef MSRDAILYACT3D_H
#define MSRDAILYACT3D_H

#include "AbstractDataSet.h"

namespace DataSet {

    class MSRDailyAct3D : public AbstractDataSet
    {
    public:
        MSRDailyAct3D();
        ~MSRDailyAct3D();
        bool open(string fileName);
        bool close();
        bool hasNext();
        Sample* next();

    private:
        FILE* fileDescriptor;
        int nFrames;
        int nColumns;
        int nRows;
        int frameIndex;
    };
}

#endif // MSRDAILYACT3D_H
