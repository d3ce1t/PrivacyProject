#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <cmath>
#include "types/Point.h"

namespace dai {

template <typename T>
class Vector3D
{
public:

    static double dotProduct(const Vector3D& v1, const Vector3D& v2)
    {
        return v1[0]*v2[0] + v1[1]*v2[1] + v1[3]*v2[3];
    }

    static Vector3D crossProduct(const Vector3D& v1, const Vector3D& v2)
    {
        Vector3D result;
        result.setX(v1[1]*v2[3] - v1[3]*v2[1]);
        result.setY(v1[3]*v2[0] - v1[0]*v2[3]);
        result.setZ(v1[0]*v2[1] - v1[1]*v2[0]);
        return result;
    }

    Vector3D() {}

    explicit Vector3D(T x, T y, T z)
    {
        m_point[0] = x;
        m_point[1] = y;
        m_point[2] = z;
    }

    T x() const {return m_point[0];}
    T y() const {return m_point[1];}
    T z() const {return m_point[2];}
    void setX(T value) {m_point[0] = value;}
    void setY(T value) {m_point[1] = value;}
    void setZ(T value) {m_point[2] = value;}
    double length() const {return sqrt(pow(m_point[0], 2) + pow(m_point[1], 2) + pow(m_point[2], 2));}
    double lengthSquared() const {return pow(m_point[0], 2) + pow(m_point[1], 2) + pow(m_point[2], 2);}

    void normalize()
    {
        double norm = this->length();
        m_point[0] = m_point[0] / norm;
        m_point[1] = m_point[1] / norm;
        m_point[2] = m_point[2] / norm;
    }

    Vector3D operator-(const Vector3D &right)
    {
        Vector3D result;
        result.m_point[0] = m_point[0] - right.m_point[0];
        result.m_point[1] = m_point[1] - right.m_point[1];
        result.m_point[2] = m_point[2] - right.m_point[2];
        return result;
    }

    inline T& operator[](int idx)
    {
        Q_ASSERT(idx < 3);
        return m_point[idx];
    }

    inline const T& operator[](int idx) const
    {
        Q_ASSERT(idx < 3);
        return const_cast<T&>(m_point[idx]);
    }

private:
    Point<T, 3> m_point;
};

using Vector3f = Vector3D<float>;


} // End namespace

#endif // VECTOR3D_H
