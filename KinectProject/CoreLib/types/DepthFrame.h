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
    DepthFrame(int width, int height, uint16_t *pData);
    virtual ~DepthFrame();
    DepthFrame(const DepthFrame& other);
    shared_ptr<DataFrame> clone() const;

    // Member Methods
    DistanceUnits distanceUnits() const;
    unsigned int getNumOfNonZeroPoints() const;
    void setDistanceUnits(DistanceUnits units);
    void setItem(int row, int column, uint16_t value);

    // Overriden operators
    DepthFrame& operator=(const DepthFrame& other);

private:
    unsigned int m_nNonZeroOfPoints;
    DistanceUnits m_units;
};

} // End Namespace

#endif // DEPTHFRAME_H
