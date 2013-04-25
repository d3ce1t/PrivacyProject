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

float Quaternion::getAngle() const
{
    return m_theta;
}

Quaternion& Quaternion::operator=(const Quaternion& other)
{
    this->setScalar(other.scalar());
    this->setVector(other.vector());
    return *this;
}

void Quaternion::setScalar(float value)
{
    // From q2rot function of Octave Quaternions package
    m_theta = acos(value) * 2;

    if (fabs(m_theta) > M_PI)
        m_theta = m_theta - sign (m_theta) * M_PI;

    QQuaternion::setScalar(value);
}

float Quaternion::sign(float value) const
{
    float result = 0;

    if (value < 0)
        result = -1;
    else if (value == 0)
        result = 0;
    else if (value > 0)
        result = 1;

    return result;
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
