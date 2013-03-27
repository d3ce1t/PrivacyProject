#include "MSRDailyAct3D.h"
#include <cstdio>
#include <stdint.h>
#include <iostream>

using namespace std;
using namespace DataSet;

MSRDailyAct3D::MSRDailyAct3D()
{
    fileDescriptor = NULL;
    nColumns = 0;
    nRows = 0;
    nFrames = 0;
    frameIndex = 0;
}

MSRDailyAct3D::~MSRDailyAct3D()
{
    close();
}

bool MSRDailyAct3D::open(string fileName)
{
    fileDescriptor = fopen(fileName.c_str(), "rb");

    if(fileDescriptor == NULL) {
        cerr << "Error opening file" << endl;
        return false;
    }

    fread(&nFrames, 4, 1, fileDescriptor); //read 4 bytes
    fread(&nColumns, 4, 1, fileDescriptor);
    fread(&nRows, 4, 1, fileDescriptor);

    return true;
}

bool MSRDailyAct3D::close()
{
    if (fileDescriptor != NULL) {
        fclose(fileDescriptor);
        fileDescriptor = NULL;
        nFrames = 0;
        nColumns = 0;
        nRows = 0;
        frameIndex = 0;
        return true;
    }

    return false;
}

bool MSRDailyAct3D::hasNext()
{
    if (frameIndex < nFrames)
        return true;

    return false;
}

Sample* MSRDailyAct3D::next()
{
    if (frameIndex < nFrames)
    {
        Sample* sample = new Sample(nColumns, nRows, frameIndex, Sample::Depth);

        // Read Data from File
        int r, c;
        int * tempRow = new int[nColumns];
        uint8_t* tempRowID = new uint8_t[nColumns];

        for(r=0; r<nRows; r++) {
            fread(tempRow, 4, nColumns, fileDescriptor);
            fread(tempRowID, 1, nColumns, fileDescriptor);

            for(c=0; c<nColumns; c++) {
                int temp = tempRow[c];
                sample->SetItem(r, c, temp);
            }
        }

        delete[] tempRow;
        tempRow = NULL;

        delete[] tempRowID;
        tempRowID = NULL;

        frameIndex++;
        return sample;
    }
    else {
        close();
    }

    return NULL;
}
