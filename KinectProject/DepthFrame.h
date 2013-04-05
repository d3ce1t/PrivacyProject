#ifndef DEPTHFRAME_H
#define DEPTHFRAME_H

#include "DataFrame.h"

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
    explicit DepthFrame(int width, int height);
    DepthFrame(const DepthFrame& other);
    virtual ~DepthFrame();

    int maxValue() const;
    int minValue() const;

    int getWidth() const;
    int getHeight() const;
    int getItem(int row, int column) const;
    void setIndex(int index);
    void setItem(int row, int column, int value);

    DepthFrame& operator=(const DepthFrame& other);

private:
    int m_width;
    int m_height;
    int m_index;
    int* m_data;
};

}

#endif // DEPTHFRAME_H
