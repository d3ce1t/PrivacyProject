#include "Point3f.h"
#include <limits>
#include <cmath>

namespace dai {

Point3f::Point3f()
{
    m_x = m_y = m_z = 0.0f;
}

Point3f::Point3f(double x, double y, double z)
{
    this->set(x, y, z);
}

Point3f::Point3f(const Point3f& other)
{
    *this = other;
}

void Point3f::set(double x, double y, double z)
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

double Point3f::x() const
{
    return m_x;
}

double Point3f::y() const
{
    return m_y;
}

double Point3f::z() const
{
    return m_z;
}

//
// Static methods
//

double Point3f::distance(const Point3f& point1, const Point3f& point2, DistanceType option)
{
    double result = std::numeric_limits<double>::max();

    if (option == DISTANCE_EUCLIDEAN) {
        result = euclideanDistance(point1, point2);
    }

    return result;
}

double Point3f::euclideanDistance(const Point3f& point1, const Point3f& point2)
{
    double dx = point2.m_x - point1.m_x;
    double dy = point2.m_y - point1.m_y;
    double dz = point2.m_z - point1.m_z;
    return sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
}

} // End Namespace
