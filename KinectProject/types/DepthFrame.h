#ifndef DEPTHFRAME_H
#define DEPTHFRAME_H

#include "DataFrame.h"
#include <stdint.h>
#include <QMap>

namespace dai {

/**
 * @brief The DepthFrame class
 *
 * A depth frame stores a distance measure in each of his x,y coordinates. This
 * distance isn't normalized and could be whatever a dataset wants to.
 *
 */
class DepthFrame : public DataFrame
{
public:
    // Static Class Methods
    static void calculateHistogram(QMap<float, float> &pHistogram, const DepthFrame &frame);
    static int maxValue(const DepthFrame& frame);
    static int minValue(const DepthFrame& frame);

    // Constructor, Destructors and Copy Constructor
    explicit DepthFrame();
    explicit DepthFrame(int width, int height);
    DepthFrame(const DepthFrame& other);
    virtual ~DepthFrame();

    // Member Methods
    int getWidth() const;
    int getHeight() const;
    unsigned int getNumberOfNonZeroPoints();
    float getItem(int row, int column) const;
    void setItem(int row, int column, float value);
    float *getDataPtr();

    // Overriden Operators
    DepthFrame& operator=(const DepthFrame& other);

private:
    int m_width;
    int m_height;
    float* m_data;
    uint8_t* m_skIDVals;
    unsigned int m_nNonZeroOfPoints;
};

}

#endif // DEPTHFRAME_H
