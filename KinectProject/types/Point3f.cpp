#include "Point3f.h"
#include <limits>
#include <cmath>

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

//
// Static methods
//

float Point3f::distance(const Point3f& point1, const Point3f& point2, DistanceType option)
{
    float result = std::numeric_limits<float>::max();

    if (option == DISTANCE_EUCLIDEAN) {
        result = euclideanDistance(point1, point2);
    }

    return result;
}

float Point3f::euclideanDistance(const Point3f& point1, const Point3f& point2)
{
    float dx = point2.m_x - point1.m_x;
    float dy = point2.m_y - point1.m_y;
    float dz = point2.m_z - point1.m_z;
    return sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
}

} // End Namespace
