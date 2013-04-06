#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "DepthFrame.h"

namespace dai {

void calculateHistogram(float* pHistogram, int histogramSize, const dai::DepthFrame& frame)
{
    // Calculate the accumulative histogram (the yellow display...)
    memset(pHistogram, 0, histogramSize*sizeof(float));

    unsigned int nNumberOfPoints = 0;

    // Count how may points there are in a given depth
    for (int y = 0; y < frame.getHeight(); ++y)
    {
        for (int x = 0; x < frame.getWidth(); ++x)
        {
            int distance = frame.getItem(y, x);

            if (distance != 0) {
                pHistogram[distance]++;
                nNumberOfPoints++;
            }
        }
    }

    // Accumulate in the given depth all the points of previous depth layers
    for (int nIndex=1; nIndex<histogramSize; nIndex++)
    {
        pHistogram[nIndex] += pHistogram[nIndex-1];
    }

    // Normalize (0% -> 256 color value, whereas 100% -> 0 color value)
    // In other words, near objects are brighter than far objects
    if (nNumberOfPoints)
    {
        for (int nIndex=1; nIndex<histogramSize; nIndex++)
        {
            pHistogram[nIndex] = (255 * (1.0f - (pHistogram[nIndex] / nNumberOfPoints)));
        }
    }
}

/*void calculateHistogram(float* pHistogram, int histogramSize, const openni::VideoFrameRef& frame)
{
    const openni::DepthPixel* pDepth = (const openni::DepthPixel*)frame.getData();

    // Calculate the accumulative histogram (the yellow display...)
    memset(pHistogram, 0, histogramSize*sizeof(float));
    memset(m_pDepthHistAux, 0, histogramSize*sizeof(float));

    int restOfRow = frame.getStrideInBytes() / sizeof(openni::DepthPixel) - frame.getWidth();
    int height = frame.getHeight();
    int width = frame.getWidth();

    unsigned int nNumberOfPoints = 0;

    // Count how may points there are in a given depth
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x, ++pDepth)
        {
            if (*pDepth != 0)
            {
                pHistogram[*pDepth]++;
                nNumberOfPoints++;
            }
        }
        // Skip rest of row (in case it exists)
        pDepth += restOfRow;
    }

    // Accumulate in the given depth all the points of previous depth layers
    for (int nIndex=1; nIndex<histogramSize; nIndex++)
    {
        pHistogram[nIndex] += pHistogram[nIndex-1];
    }

    // Normalize (0% -> 256 color value, whereas 100% -> 0 color value)
    // In other words, near objects are brighter than far objects
    if (nNumberOfPoints)
    {
        for (int nIndex=1; nIndex<histogramSize; nIndex++)
        {
            pHistogram[nIndex] = (255 * (1.0f - (pHistogram[nIndex] / nNumberOfPoints)));
        }

        memcpy(m_pDepthHistAux, pHistogram, histogramSize*sizeof(float));
    }
}*/


} // End Namespace

#endif // FUNCTIONS_H
