#ifndef DEPTHFRAME_H
#define DEPTHFRAME_H

#include "DataFrame.h"
#include <stdint.h>

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
    static void calculateHistogram(float* pHistogram, int histogramSize, const DepthFrame& frame);
    static int maxValue(const DepthFrame& frame);
    static int minValue(const DepthFrame& frame);

    // Constructor, Destructors and Copy Constructor
    explicit DepthFrame(int width, int height);
    DepthFrame(const DepthFrame& other);
    virtual ~DepthFrame();

    // Member Methods
    int getWidth() const;
    int getHeight() const;
    int getItem(int row, int column) const;
    void setItem(int row, int column, int value);

    // Overriden Operators
    DepthFrame& operator=(const DepthFrame& other);

private:
    int m_width;
    int m_height;
    int* m_data;
    uint8_t* m_skIDVals;
};

}

#endif // DEPTHFRAME_H
