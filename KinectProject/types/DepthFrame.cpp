#include "DepthFrame.h"
#include <cstring>


using namespace dai;

DepthFrame::DepthFrame()
{
    m_width = 0;
    m_height = 0;
    m_data = 0;
    m_skIDVals = 0;
}

DepthFrame::DepthFrame(int width, int height)
{
    m_width = width;
    m_height = height;
    m_data = new int[width * height];
    m_skIDVals = new uint8_t[width * height];
}

DepthFrame::DepthFrame(const DepthFrame& other)
{
    m_width = other.m_width;
    m_height = other.m_height;
    m_index = other.m_index;
    m_data = new int[m_width * m_height];

    int r,c;

    for(r=0; r<other.m_height; r++) {
        for(c=0; c<other.m_width; c++)
        {
            this->setItem(r,c, other.getItem(r, c));
        }
    }
}

DepthFrame& DepthFrame::operator=(const DepthFrame& other)
{
    // If want to reuse m_data memory. So, if size isn't correct to store new frame
    // I need to create another one.
    if (m_data == 0 || m_width != other.m_width || m_height != other.m_height)
    {
        if (m_data != 0) {
            delete[] m_data;
        }

        m_width = other.m_width;
        m_height = other.m_height;
        m_data = new int[m_width * m_height];
    }

    memset(m_data, 0, m_width * m_height * sizeof(int));
    m_index = other.m_index;

    int r,c;

    for(r=0; r<other.m_height; r++) {
        for(c=0; c<other.m_width; c++)
        {
            this->setItem(r,c, other.getItem(r, c));
        }
    }

    return *this;
}

DepthFrame::~DepthFrame()
{
    if (m_data != 0) {
        delete[] m_data;
    }

    if (m_skIDVals != 0) {
        delete[] m_skIDVals;
    }
}

int DepthFrame::getWidth() const
{
    return m_width;
}

int DepthFrame::getHeight() const
{
    return m_height;
}

int DepthFrame::getItem(int row, int column) const
{
    if (row < 0 || row >= m_height || column < 0 || column >= m_width )
        throw 1;

    return m_data[row * m_width + column] ;
}

void DepthFrame::setItem(int row, int column, int value)
{
    if (row < 0 || row >= m_height || column < 0 || column >= m_width )
        throw 1;

    m_data[row * m_width + column] = value;
}

//
// Static Class Methods
//

void DepthFrame::calculateHistogram(float* pHistogram, int histogramSize, const DepthFrame& frame)
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

int DepthFrame::maxValue(const DepthFrame& frame)
{
    int r,c;
    int bestValue = 0;

    for(r=0; r<frame.getHeight(); r++)
    {
        for(c=0; c<frame.getWidth(); c++)
        {
            int temp = frame.getItem(r,c);

            if(temp > bestValue)
            {
                bestValue = temp;
            }
        }
    }

    return bestValue;
}

int DepthFrame::minValue(const DepthFrame& frame)
{
    int r,c;
    int bestValue = 4096;

    for(r=0; r<frame.getHeight(); r++)
    {
        for(c=0; c<frame.getWidth(); c++)
        {
            int temp = frame.getItem(r,c);

            if(temp != 0 && temp < bestValue)
            {
                bestValue = temp;
            }
        }
    }

    return bestValue;
}
