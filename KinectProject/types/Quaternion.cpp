#include "Quaternion.h"
#include <cmath>

namespace dai {

Quaternion::Quaternion()
    : QObject(0)
{

}

Quaternion::Quaternion(const Quaternion& other)
    : QObject(0), QQuaternion(other.scalar(), other.vector())
{
}

Quaternion::~Quaternion()
{

}

Quaternion& Quaternion::operator=(const Quaternion& other)
{
    this->setScalar(other.scalar());
    this->setVector(other.vector());
    return *this;
}


Quaternion Quaternion::getRotationBetween(const QVector3D& v1, const QVector3D& v2)
{
    float k_cos_theta = QVector3D::dotProduct(v1, v2);
    //double k = sqrt( v1.lengthSquared() * v2.lengthSquared() );
    float k = 1; // Unit vectors!
    Quaternion result;

    if (k_cos_theta / k != -1)
    {
        float scalarPart = k + k_cos_theta;
        QVector3D vectorialPart = QVector3D::crossProduct(v1, v2); // Not unit vector
        result.setScalar(scalarPart);
        result.setVector(vectorialPart);
        result.normalize();
    }
    else
    {
        // 180 degree rotation around any axis (y-axis used here)
        result.setScalar(0);
        result.setVector(0, 1, 0);
    }

    return result;
}

} // End Namespace
