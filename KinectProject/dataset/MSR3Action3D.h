#ifndef MSR3ACTION3D_H
#define MSR3ACTION3D_H

#include "AbstractDataSet.h"

namespace DataSet {

    class MSR3Action3D : public AbstractDataSet
    {
    public:
        MSR3Action3D();
        ~MSR3Action3D();
        bool open(string fileName);
        bool close();
        bool hasNext();
        Sample* next();
        //DataSetInfo getInfo();

    private:
        FILE* fileDescriptor;
        int nFrames;
        int nColumns;
        int nRows;
        int frameIndex;
    };
}

#endif // MSR3ACTION3D_H
