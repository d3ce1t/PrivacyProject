#ifndef DEPTHFRAME_H
#define DEPTHFRAME_H

#include "GenericFrame.h"
#include "types/Enums.h"
#include <stdint.h>
#include <QMap>
#include <fstream>

namespace dai {

/**
 * @brief The DepthFrame class
 *
 * A depth frame stores a distance measure in each of his x,y coordinates. This
 * distance must be provided in meters.
 */

class DepthFrame : public GenericFrame<uint16_t, DataFrame::Depth>
{
public:
    // Static Class Methods
    static void calculateHistogram(QMap<uint16_t, float> &pHistogram, const DepthFrame &frame);

    // Constructors
    DepthFrame();
    DepthFrame(int width, int height);
    DepthFrame(int width, int height, uint16_t *pData, uint stride = 0);
    DepthFrame(const DepthFrame& other);
    shared_ptr<DataFrame> clone() const;

    // Member Methods
    DistanceUnits distanceUnits() const {return m_units;}
    void setDistanceUnits(DistanceUnits units) {m_units = units;}
    shared_ptr<DepthFrame> subFrame(int row, int column, int width, int height) const;
    shared_ptr<DepthFrame> subFrame(const BoundingBox& bb) const {
        return subFrame(bb.getMin()[1], bb.getMin()[0], bb.size().width(), bb.size().height());
    }

    // Overriden operators
    DepthFrame& operator=(const DepthFrame& other);

    // Extra
    void convertCoordinatesToWorld(float x, float y, float z, float* pOutX, float* pOutY) const;
    void setCameraIntrinsics(double fx_d, double cx_d, double fy_d, double cy_d);

private:
    // Depth Intrinsics
    double m_fx_d = 594.21434211923247;
    double m_fy_d = 591.04053696870778;
    double m_cx_d = 320.0;
    double m_cy_d = 240.0;
    DistanceUnits m_units;
};

} // End Namespace

#endif // DEPTHFRAME_H
