#include "DepthFrame.h"
#include <cstring>
#include <limits>
#include "../dataset/DataInstance.h"
#include <QDebug>

using namespace dai;

DepthFrame::DepthFrame()
{
    m_width = 0;
    m_height = 0;
    m_data = 0;
    m_label = 0;
    m_skIDVals = 0;
    m_nNonZeroOfPoints = 0;
}

DepthFrame::DepthFrame(int width, int height)
{
    m_width = width;
    m_height = height;
    m_data = new float[width * height];
    m_label = new short int[width * height];
    m_skIDVals = new uint8_t[width * height];
    memset(m_data, 0, width*height*sizeof(float));
    memset(m_label, 0, width*height*sizeof(short int));
    memset(m_skIDVals, 0, width*height*sizeof(uint8_t));
    m_nNonZeroOfPoints = 0;
}

DepthFrame::DepthFrame(const DepthFrame& other)
    : DataFrame(other)
{
    m_width = other.m_width;
    m_height = other.m_height;
    m_index = other.m_index;
    m_nNonZeroOfPoints = other.m_nNonZeroOfPoints;
    m_data = new float[m_width * m_height];
    m_label = new short int[m_width * m_height];
    memcpy(m_data, other.m_data, m_width * m_height * sizeof(float));
    memcpy(m_label, other.m_label, m_width * m_height * sizeof(short int));
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
        m_label = new short int[m_width * m_height];
    }

    memcpy(m_data, other.m_data, m_width * m_height * sizeof(float));
    memcpy(m_label, other.m_label, m_width * m_height * sizeof(short int));
    m_index = other.m_index;
    m_nNonZeroOfPoints = other.m_nNonZeroOfPoints;
    return *this;
}

DepthFrame::~DepthFrame()
{
    if (m_data != 0) {
        delete[] m_data;
    }

    if (m_label != 0) {
        delete[] m_label;
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

unsigned int DepthFrame::getNumOfNonZeroPoints() const
{
    return m_nNonZeroOfPoints;
}

float DepthFrame::getItem(int row, int column) const
{
    if (row < 0 || row >= m_height || column < 0 || column >= m_width )
        throw 1;

    return m_data[row * m_width + column];
}

short int DepthFrame::getLabel(int row, int column) const
{
    if (row < 0 || row >= m_height || column < 0 || column >= m_width )
        throw 1;

    return m_label[row * m_width + column];
}

void DepthFrame::setItem(int row, int column, float value)
{
    if (row < 0 || row >= m_height || column < 0 || column >= m_width )
        throw 1;

    int index = row * m_width + column;
    float current_value = m_data[index];

    if (value != 0 && current_value == 0) {
        m_nNonZeroOfPoints++;
    } else if (value == 0 && current_value != 0) {
        m_nNonZeroOfPoints--;
    }

    m_data[index] = value;
}

void DepthFrame::setItem(int row, int column, float value, short int label)
{
    if (row < 0 || row >= m_height || column < 0 || column >= m_width )
        throw 1;

    int index = row * m_width + column;
    float current_value = m_data[index];

    if (value != 0 && current_value == 0) {
        m_nNonZeroOfPoints++;
    } else if (value == 0 && current_value != 0) {
        m_nNonZeroOfPoints--;
    }

    m_data[index] = value;
    m_label[index] = label;
}

const float *DepthFrame::getDataPtr() const
{
    return m_data;
}

void DepthFrame::toArray(float dst[][3], int size) const
{
    float *dstPtr = &dst[0][0];
    float *endPtr = &dst[size-1][2];

    for (int i=0; i<m_height && dstPtr < endPtr; ++i)
    {
        for (int j=0; j<m_width && dstPtr < endPtr; ++j)
        {
            float distance = getItem(i, j);
            float normX = DataInstance::normalise(j, 0, m_width-1, -1, 1);
            float normY = -DataInstance::normalise(i, 0, m_height-1, -1, 1);

            *dstPtr++ = normX;
            *dstPtr++ = normY;
            *dstPtr++ = distance;
        }
    }
}

void DepthFrame::write(std::ofstream& of) const
{
    char* buffer = (char *) m_data;
    of.write(buffer, m_width * m_height * sizeof(float));
    buffer = (char *) m_label;
    of.write(buffer, m_width * m_height * sizeof(short int));
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

float DepthFrame::maxValue(const DepthFrame& frame)
{
    int r,c;
    float bestValue = std::numeric_limits<float>::min();

    for(r=0; r<frame.m_height; r++)
    {
        for(c=0; c<frame.m_width; c++)
        {
            float temp = frame.m_data[r * frame.m_width + c];

            if(temp > bestValue)
            {
                bestValue = temp;
            }
        }
    }

    return bestValue;
}

float DepthFrame::minValue(const DepthFrame& frame)
{
    int r,c;
    float bestValue = std::numeric_limits<float>::max();

    for(r=0; r<frame.m_height; r++)
    {
        for(c=0; c<frame.m_width; c++)
        {
            float temp = frame.m_data[r * frame.m_width + c];

            if(temp < bestValue)
            {
                bestValue = temp;
            }
        }
    }

    return bestValue;
}
