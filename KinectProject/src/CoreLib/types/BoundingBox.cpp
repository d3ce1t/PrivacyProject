#include "BoundingBox.h"

namespace dai {

BoundingBox::BoundingBox(const Point3f& min, const Point3f& max)
{
    m_min = min;
    m_max = max;
    m_size.setWidth(max.val(0) - min.val(0));
    m_size.setHeight(max.val(1) - min.val(1));
}

const Point3f& BoundingBox::getMin() const
{
    return m_min;
}

const Point3f& BoundingBox::getMax() const
{
    return m_max;
}

const QSize& BoundingBox::size() const
{
    return m_size;
}

} // End Namespace
