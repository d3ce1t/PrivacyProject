#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "Point.h"
#include <QSize>

namespace dai {

class BoundingBox
{
public:
    BoundingBox(const Point3f& min, const Point3f& max);
    const Point3f& getMin() const;
    const Point3f& getMax() const;
    const QSize& size() const;

private:
    Point3f m_min;
    Point3f m_max;
    QSize m_size;
};

} // End Namespace

#endif // BOUNDINGBOX_H
