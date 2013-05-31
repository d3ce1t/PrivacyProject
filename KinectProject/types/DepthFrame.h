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
 * distance isn't normalized and could be whatever a dataset wants to.
 *
 */

class DepthFrame : public GenericFrame<float>
{
public:
    // Static Class Methods
    static void calculateHistogram(QMap<float, float> &pHistogram, const DepthFrame &frame);

    // Constructors
    DepthFrame();
    DepthFrame(int width, int height);
    virtual ~DepthFrame();
    DepthFrame(const DepthFrame& other);

    // Member Methods
    unsigned int getNumOfNonZeroPoints() const;
    void setItem(int row, int column, float value);
    void setItem(int row, int column, float value, short int label);

    // Overriden operators
    DepthFrame& operator=(const DepthFrame& other);

private:
    unsigned int m_nNonZeroOfPoints;
};

} // End Namespace

#endif // DEPTHFRAME_H
