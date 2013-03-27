#include <math.h>
#include "dataset/Sample.h"
#include <cstddef>

using namespace DataSet;

Sample::Sample() {
    rawData = NULL;
    nColumns = 0;
    nRows = 0;
    type = NO_TYPE;
    frameIndex = 0;
}

Sample::Sample(int ncols, int nrows, int frameIndex, Sample::Type type)
{
    this->type = type;
    nRows = nrows;
    nColumns = ncols;
    rawData = new int[nColumns * nRows];
    this->frameIndex = frameIndex;

}

Sample::Sample(const Sample& other)
{
    nRows = other.GetNRows();
    nColumns = other.GetNCols();
    frameIndex = other.getFrameIndex();
    rawData = new int[nColumns * nRows];

    int r,c;

    for(r=0; r<other.GetNRows(); r++) {
        for(c=0; c<other.GetNCols(); c++)
        {
            float temp = other.GetItem(r,c);
            this->SetItem(r,c, temp);
        }
    }
}

Sample& Sample::operator=(const Sample& other)
{
    if (rawData != NULL) {
        delete[] rawData;
        rawData = NULL;
    }

    nRows = other.GetNRows();
    nColumns = other.GetNCols();
    frameIndex = other.getFrameIndex();
    rawData = new int[nColumns * nRows];


    int r,c;

    for(r=0; r<other.GetNRows(); r++) {
        for(c=0; c<other.GetNCols(); c++)
        {
            float temp = other.GetItem(r,c);
            this->SetItem(r,c, temp);
        }
    }

    return *this;
}

Sample::~Sample(void)
{
    if(rawData != NULL) {
        delete[] rawData;
        rawData = NULL;
        nRows = 0;
        nColumns = 0;
        frameIndex = 0;
    }
}

void Sample::SetItem(int r, int c, int val)
{
    rawData[r * nColumns + c] = val;
}

int Sample::GetItem(int r, int c) const
{
    return rawData[r * nColumns + c] ;
}

int Sample::MaxValue() const
{
    int r,c;
    int bestValue = 0;

    for(r=0; r<GetNRows(); r++)
    {
        for(c=0; c<GetNCols(); c++)
        {
            int temp = GetItem(r,c);

            if(temp > bestValue)
            {
                bestValue = temp;
            }
        }
    }
    return bestValue;
}

int Sample::MinValue() const
{
    int r,c;
    int bestValue = 4096;

    for(r=0; r<GetNRows(); r++)
    {
        for(c=0; c<GetNCols(); c++)
        {
            int temp = GetItem(r,c);

            if(temp != 0 && temp < bestValue)
            {
                bestValue = temp;
            }
        }
    }
    return bestValue;
}

int Sample::NumberOfNonZeroPoints() const
{
	int count=0;
	int r,c;
	for(r=0; r<GetNRows(); r++)
	{
		for(c=0; c<GetNCols(); c++)
		{
            int temp = GetItem(r,c);
			if(temp != 0)
			{
				count++;
			}
		}
	}
	return count;
}

float Sample::AvgNonZeroDepth() const
{
	int count=0;
	float sum = 0;
	int r,c;
	for(r=0; r<GetNRows(); r++)
	{
		for(c=0; c<GetNCols(); c++)
		{
            int temp = GetItem(r,c);
			if(temp != 0)
			{
				count++;
				sum += temp;
			}
		}
	}
	if(count == 0)
		return 0;
	else
		return sum/count;
}
