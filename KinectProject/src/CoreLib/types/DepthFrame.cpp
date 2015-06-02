#include "DepthFrame.h"
#include <cstring>
#include <limits>
#include <QDebug>
#include "Utils.h"

namespace dai {

DepthFrame::DepthFrame()
{
    m_units = DISTANCE_METERS;
}

DepthFrame::DepthFrame(int width, int height)
    : GenericFrame<uint16_t,DataFrame::Depth>(width, height)
{
     m_units = DISTANCE_METERS;
}

DepthFrame::DepthFrame(int width, int height, uint16_t *pData, uint stride)
    : GenericFrame<uint16_t,DataFrame::Depth>(width, height, pData, stride)
{
     m_units = DISTANCE_METERS;
}

DepthFrame::DepthFrame(const DepthFrame& other)
    : GenericFrame<uint16_t,DataFrame::Depth>(other)
{
    m_units = other.m_units;
    m_fx_d = other.m_fx_d;
    m_fy_d = other.m_fy_d;
    m_cx_d = other.m_cx_d;
    m_cy_d = other.m_cy_d;
}

shared_ptr<DataFrame> DepthFrame::clone() const
{
    return shared_ptr<DataFrame>(new DepthFrame(*this));
}

DepthFrame& DepthFrame::operator=(const DepthFrame& other)
{
    GenericFrame<uint16_t,DataFrame::Depth>::operator=(other);
    m_units = other.m_units;
    m_fx_d = other.m_fx_d;
    m_fy_d = other.m_fy_d;
    m_cx_d = other.m_cx_d;
    m_cy_d = other.m_cy_d;
    return *this;
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
    for (int i = 0; i < frame.height(); ++i)
    {
        const uint16_t* pDepth = frame.getRowPtr(i);

        for (int j = 0; j < frame.width(); ++j)
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

shared_ptr<DepthFrame> DepthFrame::subFrame(int row, int column, int width, int height) const
{
    shared_ptr<DepthFrame> result = make_shared<DepthFrame>();
    shared_ptr<GenericFrame> source = GenericFrame::subFrame(row, column, width, height);
    result->setDataPtr(source->width(), source->height(), source->getDataPtr(), source->getStride());
    result->setOffset(source->offset());
    result->m_units = m_units;
    return result;
}

void DepthFrame::setCameraIntrinsics(double fx_d, double cx_d, double fy_d, double cy_d){
    m_fx_d = fx_d;
    m_cx_d = cx_d;
    m_fy_d = fy_d;
    m_cy_d = cy_d;
}

void DepthFrame::convertCoordinatesToWorld(float x, float y, float z, float* pOutX, float* pOutY) const
{
    x = x + offset()[0];
    y = y + offset()[1];
    *pOutX = (x - m_cx_d) * z / m_fx_d;
    *pOutY = (y - m_cy_d) * z / m_fy_d;
}

} // End Namespace
