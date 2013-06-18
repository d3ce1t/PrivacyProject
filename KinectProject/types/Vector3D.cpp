#include "Vector3D.h"
#include <cmath>

namespace dai {

Vector3D::Vector3D()
{
    m_x = 0.0;
    m_y = 0.0;
    m_z = 0.0;
}

Vector3D::Vector3D(double x, double y, double z)
{
    m_x = x;
    m_y = y;
    m_z = z;
}

double Vector3D::x() const
{
    return m_x;
}

double Vector3D::y() const
{
    return m_y;
}

double Vector3D::z() const
{
    return m_z;
}

void Vector3D::setX(double value)
{
    m_x = value;
}

void Vector3D::setY(double value)
{
    m_y = value;
}

void Vector3D::setZ(double value)
{
    m_z = value;
}

double Vector3D::length() const
{
    return sqrt(pow(m_x, 2) + pow(m_y, 2) + pow(m_z, 2));
}

double Vector3D::lengthSquared() const
{
    return pow(m_x, 2) + pow(m_y, 2) + pow(m_z, 2);
}

void Vector3D::normalize()
{
    double norm = this->length();
    m_x = m_x / norm;
    m_y = m_y / norm;
    m_z = m_z / norm;
}

double Vector3D::dotProduct(const Vector3D& v1, const Vector3D& v2)
{
    return v1.x()*v2.x() + v1.y()*v2.y() + v1.z()*v2.z();
}

Vector3D Vector3D::crossProduct(const Vector3D& v1, const Vector3D& v2)
{
    Vector3D result;
    result.setX(v1.y()*v2.z() - v1.z()*v2.y());
    result.setY(v1.z()*v2.x() - v1.x()*v2.z());
    result.setZ(v1.x()*v2.y() - v1.y()*v2.x());
    return result;
}

} // End namespace
