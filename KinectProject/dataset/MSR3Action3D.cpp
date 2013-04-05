#include "MSR3Action3D.h"
#include <iostream>
#include <cstdio>

using namespace std;

/*MSR3Action3D::MSR3Action3D()
{
    fileDescriptor = NULL;
    nColumns = 0;
    nRows = 0;
    nFrames = 0;
    frameIndex = 0;
}

MSR3Action3D::~MSR3Action3D()
{
    close();
}

bool MSR3Action3D::open(string fileName)
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

bool MSR3Action3D::close()
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

bool MSR3Action3D::hasNext()
{
    if (frameIndex < nFrames)
        return true;

    return false;
}

Sample* MSR3Action3D::next()
{
    if (frameIndex < nFrames)
    {
        Sample* sample = new Sample(nColumns, nRows, frameIndex, Sample::Depth);

        // Read Data from File
        int r, c;
        int* tempRow = new int[nColumns];

        for(r=0; r<nRows; r++) {
            fread(tempRow, 4, nColumns, fileDescriptor);

            for(c=0; c<nColumns; c++) {
                int temp = tempRow[c];
                sample->SetItem(r, c, temp);
            }
        }

        delete[] tempRow;
        tempRow = NULL;



        frameIndex++;
        return sample;
    }
    else {
        close();
    }

    return NULL;
}*/

/*DataSetInfo MSR3Action3D::getInfo()
{

}*/
