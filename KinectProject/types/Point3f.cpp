#include "Point3f.h"

namespace dai {

Point3f::Point3f()
{
    m_x = m_y = m_z = 0.0f;
}

Point3f::Point3f(float x, float y, float z)
{
    this->set(x, y, z);
}

Point3f::Point3f(const Point3f& other)
{
    *this = other;
}

void Point3f::set(float x, float y, float z)
{
    m_x = x;
    m_y = y;
    m_z = z;
}

Point3f& Point3f::operator=(const Point3f& other)
{
    set(other.m_x, other.m_y, other.m_z);

    return *this;
}

bool Point3f::operator==(const Point3f& other) const
{
    return m_x == other.m_x && m_y == other.m_y && m_z == other.m_z;
}

bool Point3f::operator!=(const Point3f& other) const
{
    return !operator==(other);
}

float Point3f::x() const
{
    return m_x;
}

float Point3f::y() const
{
    return m_y;
}

float Point3f::z() const
{
    return m_z;
}

} // End Namespace
