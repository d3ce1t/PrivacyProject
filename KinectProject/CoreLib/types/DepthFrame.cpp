#include "DepthFrame.h"
#include <cstring>
#include <limits>
#include <QDebug>

namespace dai {

DepthFrame::DepthFrame()
{
    m_units = METERS;
    m_nNonZeroOfPoints = 0;
}

DepthFrame::DepthFrame(int width, int height)
    : GenericFrame<float,DataFrame::Depth>(width, height)
{
    m_nNonZeroOfPoints = 0;
}

DepthFrame::DepthFrame(int width, int height, float *pData)
    : GenericFrame<float,DataFrame::Depth>(width, height, pData)
{
     m_nNonZeroOfPoints = 0;
}

DepthFrame::~DepthFrame()
{
    m_nNonZeroOfPoints = 0;
}

DepthFrame::DepthFrame(const DepthFrame& other)
    : GenericFrame<float,DataFrame::Depth>(other)
{
    m_nNonZeroOfPoints = other.m_nNonZeroOfPoints;
}

shared_ptr<DataFrame> DepthFrame::clone() const
{
    return shared_ptr<DataFrame>(new DepthFrame(*this));
}

DepthFrame& DepthFrame::operator=(const DepthFrame& other)
{
    GenericFrame<float,DataFrame::Depth>::operator=(other);
    m_nNonZeroOfPoints = other.m_nNonZeroOfPoints;
    return *this;
}

unsigned int DepthFrame::getNumOfNonZeroPoints() const
{
    return m_nNonZeroOfPoints;
}

DepthFrame::DistanceUnits DepthFrame::distanceUnits() const
{
    return m_units;
}

void DepthFrame::setItem(int row, int column, float value)
{
    uint16_t current_value = GenericFrame<float,DataFrame::Depth>::getItem(row, column);
    GenericFrame<float,DataFrame::Depth>::setItem(row, column, value);

    if (value != 0 && current_value == 0) {
        m_nNonZeroOfPoints++;
    } else if (value == 0 && current_value != 0) {
        m_nNonZeroOfPoints--;
    }
}

//
// Static Class Methods
//
void DepthFrame::calculateHistogram(QMap<float, float> &pHistogram, const DepthFrame& frame)
{
    pHistogram.clear();

    int width = frame.getWidth();
    int height = frame.getHeight();

    // Count how may points there are in a given depth
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float distance = frame.getItem(y, x);

            if (distance != 0) {
                if (pHistogram.contains(distance)) {
                    pHistogram[distance]++;
                } else {
                    pHistogram.insert(distance, 1);
                }
            }
        }
    }

    // Accumulate in the given depth all the points of previous depth layers
    QMapIterator<float, float> it(pHistogram);
    uint16_t previousKey;
    uint16_t currentKey;

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
    if (frame.m_nNonZeroOfPoints)
    {
        QMapIterator<float, float> it(pHistogram);
        float currentKey;

        while (it.hasNext()) {
            it.next();
            currentKey = it.key();
            pHistogram[currentKey] = 1.0f - (pHistogram[currentKey] / frame.m_nNonZeroOfPoints);
        }
    }
}

} // End Namespace
