#include "DepthFrame.h"
#include <cstring>
#include <limits>
#include <QDebug>

using namespace dai;

DepthFrame::DepthFrame()
{
    m_width = 0;
    m_height = 0;
    m_data = 0;
    m_skIDVals = 0;
    m_nNonZeroOfPoints = 0;
}

DepthFrame::DepthFrame(int width, int height)
{
    m_width = width;
    m_height = height;
    m_data = new float[width * height];
    m_skIDVals = new uint8_t[width * height];
    m_nNonZeroOfPoints = 0;
}

DepthFrame::DepthFrame(const DepthFrame& other)
{
    m_width = other.m_width;
    m_height = other.m_height;
    m_index = other.m_index;
    m_nNonZeroOfPoints = other.m_nNonZeroOfPoints;
    m_data = new float[m_width * m_height];
    memcpy(m_data, other.m_data, m_width * m_height * sizeof(float));
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
        m_data = new float[m_width * m_height];
    }

    memcpy(m_data, other.m_data, m_width * m_height * sizeof(float));
    m_index = other.m_index;
    m_nNonZeroOfPoints = other.m_nNonZeroOfPoints;
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

unsigned int DepthFrame::getNumberOfNonZeroPoints()
{
    unsigned int nNonZeroPoints = 0;

    if (m_nNonZeroOfPoints != 0) {
        nNonZeroPoints = m_nNonZeroOfPoints;
    } else {
        // Count how may points there are in a given depth
        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                float distance = m_data[y * m_width + x];

                if (distance != 0) {
                    nNonZeroPoints++;
                }
            }
        }

        m_nNonZeroOfPoints = nNonZeroPoints;
    }

    return nNonZeroPoints;
}

float DepthFrame::getItem(int row, int column) const
{
    if (row < 0 || row >= m_height || column < 0 || column >= m_width )
        throw 1;

    return m_data[row * m_width + column];
}

void DepthFrame::setItem(int row, int column, float value)
{
    if (row < 0 || row >= m_height || column < 0 || column >= m_width )
        throw 1;

    m_nNonZeroOfPoints = 0;
    m_data[row * m_width + column] = value;
}

float *DepthFrame::getDataPtr()
{
    return m_data;
}

//
// Static Class Methods
//

void DepthFrame::calculateHistogram(QMap<float, float> &pHistogram, /*int histogramSize,*/ const DepthFrame& frame)
{
    int nNumberOfPoints = 0;

    pHistogram.clear();

    // Count how may points there are in a given depth
    for (int y = 0; y < frame.m_height; ++y)
    {
        for (int x = 0; x < frame.m_width; ++x)
        {
            float distance = frame.m_data[y * frame.m_width + x];

            if (distance != 0) {
                if (pHistogram.contains(distance)) {
                    pHistogram[distance]++;
                } else {
                    pHistogram.insert(distance, 1);
                }
                nNumberOfPoints++;
            }
        }
    }

    // Accumulate in the given depth all the points of previous depth layers
    QMapIterator<float, float> it(pHistogram);
    float previousKey;
    float currentKey;

    if (it.hasNext()) {
        it.next();
        previousKey = it.key();
    }

    while (it.hasNext()) {
        it.next();
        currentKey = it.key();
        pHistogram[currentKey] += pHistogram[previousKey];
        previousKey = currentKey;
    }

    // Normalize (0% -> 256 color value, whereas 100% -> 0 color value)
    // In other words, near objects are brighter than far objects
    if (nNumberOfPoints)
    {
        QMapIterator<float, float> it(pHistogram);
        float currentKey;

        while (it.hasNext()) {
            it.next();
            currentKey = it.key();
            pHistogram[currentKey] = 1.0f - (pHistogram[currentKey] / nNumberOfPoints);
        }
    }
}

int DepthFrame::maxValue(const DepthFrame& frame)
{
    int r,c;
    int bestValue = std::numeric_limits<int>::min();

    for(r=0; r<frame.m_height; r++)
    {
        for(c=0; c<frame.m_width; c++)
        {
            int temp = frame.m_data[r * frame.m_width + c];

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
    int bestValue = std::numeric_limits<int>::max();

    for(r=0; r<frame.m_height; r++)
    {
        for(c=0; c<frame.m_width; c++)
        {
            int temp = frame.m_data[r * frame.m_width + c];

            if(temp != 0 && temp < bestValue)
            {
                bestValue = temp;
            }
        }
    }

    return bestValue;
}
