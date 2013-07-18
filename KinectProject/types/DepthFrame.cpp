#include "DepthFrame.h"
#include <cstring>
#include <limits>
#include "../dataset/DataInstance.h"
#include <QDebug>

namespace dai {

DepthFrame::DepthFrame()
    : GenericFrame<float>(DataFrame::Depth)
{
    m_nNonZeroOfPoints = 0;
}

DepthFrame::DepthFrame(int width, int height)
    : GenericFrame<float>(width, height, DataFrame::Depth)
{
    m_nNonZeroOfPoints = 0;
}

DepthFrame::~DepthFrame()
{
    m_nNonZeroOfPoints = 0;
}

DepthFrame::DepthFrame(const DepthFrame& other)
    : GenericFrame<float>(other)
{
    m_nNonZeroOfPoints = other.m_nNonZeroOfPoints;
}

shared_ptr<DataFrame> DepthFrame::clone() const
{
    return shared_ptr<DataFrame>(new DepthFrame(*this));
}

DepthFrame& DepthFrame::operator=(const DepthFrame& other)
{
    GenericFrame<float>::operator=(other);
    m_nNonZeroOfPoints = other.m_nNonZeroOfPoints;
    return *this;
}

unsigned int DepthFrame::getNumOfNonZeroPoints() const
{
    return m_nNonZeroOfPoints;
}

void DepthFrame::setItem(int row, int column, float value)
{
    float current_value = GenericFrame<float>::getItem(row, column);
    GenericFrame<float>::setItem(row, column, value);

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
