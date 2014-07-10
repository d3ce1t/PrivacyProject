#include "BoundingBox.h"

namespace dai {

BoundingBox::BoundingBox(const Point3f& min, const Point3f& max)
{
    m_min = min;
    m_max = max;
    m_size.setWidth(max.x() - min.x());
    m_size.setHeight(max.y() - min.y());
}

const Point3f BoundingBox::getMin() const
{
    return m_min;
}

const Point3f BoundingBox::getMax() const
{
    return m_max;
}

const QSize& BoundingBox::size() const
{
    return m_size;
}

} // End Namespace
