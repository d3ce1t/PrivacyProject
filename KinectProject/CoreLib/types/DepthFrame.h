#ifndef DEPTHFRAME_H
#define DEPTHFRAME_H

#include "GenericFrame.h"
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

class DepthFrame : public GenericFrame<float>
{
public:

    enum DistanceUnits {
        MILIMETERS,
        METERS
    };

    // Static Class Methods
    static void calculateHistogram(QMap<float, float> &pHistogram, const DepthFrame &frame);

    // Constructors
    DepthFrame();
    DepthFrame(int width, int height);
    DepthFrame(int width, int height, float *pData);
    virtual ~DepthFrame();
    DepthFrame(const DepthFrame& other);
    shared_ptr<DataFrame> clone() const;
    DistanceUnits distanceUnits() const;

    // Member Methods
    unsigned int getNumOfNonZeroPoints() const;
    void setItem(int row, int column, float value);

    // Overriden operators
    DepthFrame& operator=(const DepthFrame& other);

private:
    unsigned int m_nNonZeroOfPoints;
    DistanceUnits m_units;
};

} // End Namespace

#endif // DEPTHFRAME_H
