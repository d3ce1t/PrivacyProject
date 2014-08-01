#include "DepthFrame.h"
#include <cstring>
#include <limits>
#include <QDebug>
#include "Utils.h"

namespace dai {

DepthFrame::DepthFrame()
{
    m_units = METERS;
    m_nNonZeroOfPoints = 0;
}

DepthFrame::DepthFrame(int width, int height)
    : GenericFrame<uint16_t,DataFrame::Depth>(width, height)
{
    m_nNonZeroOfPoints = 0;
}

DepthFrame::DepthFrame(int width, int height, uint16_t *pData)
    : GenericFrame<uint16_t,DataFrame::Depth>(width, height, pData)
{
     m_nNonZeroOfPoints = 0;
}

DepthFrame::~DepthFrame()
{
    m_nNonZeroOfPoints = 0;
}

DepthFrame::DepthFrame(const DepthFrame& other)
    : GenericFrame<uint16_t,DataFrame::Depth>(other)
{
    m_nNonZeroOfPoints = other.m_nNonZeroOfPoints;
}

shared_ptr<DataFrame> DepthFrame::clone() const
{
    return shared_ptr<DataFrame>(new DepthFrame(*this));
}

DepthFrame& DepthFrame::operator=(const DepthFrame& other)
{
    GenericFrame<uint16_t,DataFrame::Depth>::operator=(other);
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

void DepthFrame::setDistanceUnits(DistanceUnits units)
{
    m_units = units;
}

void DepthFrame::setItem(int row, int column, uint16_t value)
{
    uint16_t current_value = GenericFrame<uint16_t,DataFrame::Depth>::getItem(row, column);
    GenericFrame<uint16_t,DataFrame::Depth>::setItem(row, column, value);

    if (value != 0 && current_value == 0) {
        m_nNonZeroOfPoints++;
    } else if (value == 0 && current_value != 0) {
        m_nNonZeroOfPoints--;
    }
}

//
// Static Class Methods
//
void DepthFrame::calculateHistogram(QMap<uint16_t, float> &pHistogram, const DepthFrame& frame)
{
    pHistogram.clear();

    int points_number = 0;
    pHistogram[0] = 0;

    // Count how may points there are in a given depth
    for (int i = 0; i < frame.getHeight(); ++i)
    {
        const uint16_t* pDepth = frame.getRowPtr(i);

        for (int j = 0; j < frame.getWidth(); ++j)
        {
            uint16_t distance = pDepth[j];

            if (distance != 0) {
                if (pHistogram.contains(distance)) {
                    pHistogram[distance]++;
                } else {
                    pHistogram.insert(distance, 1);
                }
                points_number++;
            }
        }
    }

    // Accumulate in the given depth all the points of previous depth layers
    QMapIterator<uint16_t, float> it(pHistogram);
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
    if (points_number)
    {
        QMapIterator<uint16_t, float> it(pHistogram);
        uint16_t currentKey;
        it.next(); // Skip 0 depth

        while (it.hasNext()) {
            it.next();
            currentKey = it.key();
            pHistogram[currentKey] = 255 * (1.0f - (pHistogram[currentKey] / float(points_number)));
        }
    }
}

} // End Namespace
