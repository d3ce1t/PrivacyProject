#ifndef VECTOR3D_H
#define VECTOR3D_H

namespace dai {

class Vector3D
{
public:
    static double dotProduct(const Vector3D& v1, const Vector3D& v2);
    static Vector3D crossProduct(const Vector3D& v1, const Vector3D& v2);

    Vector3D();
    explicit Vector3D(double x, double y, double z);
    double x() const;
    double y() const;
    double z() const;    
    void setX(double value);
    void setY(double value);
    void setZ(double value);
    double length() const;
    double lengthSquared() const;
    void normalize();

private:
    double m_x;
    double m_y;
    double m_z;
};

} // End namespace

#endif // VECTOR3D_H
