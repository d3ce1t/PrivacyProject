#include "Point3f.h"
#include <limits>
#include <cmath>

namespace dai {

Point3f::Point3f()
{
    m_pos[0] = m_pos[1] = m_pos[2] = 0.0f;
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
    m_pos[0] = x;
    m_pos[1] = y;
    m_pos[2] = z;
}

Point3f& Point3f::operator=(const Point3f& other)
{
    set(other.m_pos[0], other.m_pos[1], other.m_pos[2]);
    return *this;
}

bool Point3f::operator==(const Point3f& other) const
{
    return m_pos[0] == other.m_pos[0] && m_pos[1] == other.m_pos[1] && m_pos[2] == other.m_pos[2];
}

bool Point3f::operator!=(const Point3f& other) const
{
    return !operator==(other);
}

float Point3f::x() const
{
    return m_pos[0];
}

float Point3f::y() const
{
    return m_pos[1];
}

float Point3f::z() const
{
    return m_pos[2];
}

const float* Point3f::dataPtr() const
{
    return m_pos;
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
    float dx = point2.x() - point1.x();
    float dy = point2.y() - point1.y();
    float dz = point2.z() - point1.z();
    return sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
}

} // End Namespace
