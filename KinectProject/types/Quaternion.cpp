#include "Quaternion.h"
#include <cmath>
#include <iostream>

#ifndef M_PI
    #define M_PI 3.14159265359
#endif

#include <cstdlib>
#include <QDebug>
#include <QQuaternion>

using namespace std;

namespace dai {

void Quaternion::test()
{
    // Initializing seed for rand() function
    srand(time(NULL));

    QVector3D* vector = new QVector3D[200];
    qDebug() << "Generating 200 unit vectors";

    for (int i=0; i<200; ++i)
    {
        float pos_x = rand() / (float) RAND_MAX;
        float pos_y = rand() / (float) RAND_MAX;
        float pos_z = rand() / (float) RAND_MAX;

        vector[i].setX(pos_x);
        vector[i].setY(pos_y);
        vector[i].setZ(pos_z);
        vector[i].normalize();
    }

    Quaternion* q = new Quaternion[100];
    qDebug() << "Computing quaternion between pairs of vectors";

    for (int i=0; i<100; ++i)
    {
        QVector3D& v1 = vector[i*2];
        QVector3D& v2 = vector[i*2+1];
        q[i] = Quaternion::getRotationBetween(v1, v2);
    }

    qDebug() << "Validating results";

    for (int i=0; i<100; ++i)
    {
        QVector3D& v1 = vector[i*2];
        QVector3D& v2 = vector[i*2+1];
        QQuaternion rot(q[i].scalar(), q[i].vector());
        QVector3D result = rot.rotatedVector(v1);

        if (fuzzyCompare(result, v2)) {
            qDebug() << "Test " << i+1 << "\t" << "OK";
        } else {
            qDebug() << "Test " << i+1 << "\t" << "Error";
            qDebug() << "Expected" << v2;
            qDebug() << "Obtained" << result;
        }
    }

    delete[] vector;
    delete[] q;
}

bool Quaternion::fuzzyCompare(const QVector3D& v1, const QVector3D& v2)
{
    return qFuzzyCompare(v1.x(), v2.x()) && qFuzzyCompare(v1.y(), v2.y()) && qFuzzyCompare(v1.z(), v2.z());
}


Quaternion::Quaternion()
    : QObject(0)
{
    // Identity quaternion
    m_scalarPart = 1.0f;
    m_vectorialPart = QVector3D(0, 0, 0);
}

Quaternion::Quaternion(const Quaternion& other)
    : QObject(0)
{
    m_scalarPart = other.scalar();
    m_vectorialPart = other.vector();
}

Quaternion& Quaternion::operator=(const Quaternion& other)
{
    this->setScalar(other.scalar());
    this->setVector(other.vector());
    return *this;
}

void Quaternion::setScalar(float value)
{
    // Calculate the rotation angle for this quaternion
    // in order to get it cached.
    // From q2rot function of Octave Quaternions package
    m_theta = acos(value) * 2;

    if (fabs(m_theta) > M_PI)
        m_theta = m_theta - sign (m_theta) * M_PI;

    // Store the scalar part
    m_scalarPart = value;
}

void Quaternion::setVector(QVector3D vector)
{
    m_vectorialPart = vector;
}

void Quaternion::setVector(float i, float j, float k)
{
    m_vectorialPart = QVector3D(i, j, k);
}

float Quaternion::scalar() const
{
    return m_scalarPart;
}

QVector3D Quaternion::vector() const
{
    return m_vectorialPart;
}

float Quaternion::getAngle() const
{
    return m_theta;
}

float Quaternion::norm() const
{
    return sqrt(powf(m_scalarPart, 2) +
                powf(m_vectorialPart.x(), 2) +
                powf(m_vectorialPart.y(), 2) +
                powf(m_vectorialPart.z(), 2));
}

void Quaternion::normalize()
{
    float norm = this->norm();
    m_scalarPart = m_scalarPart / norm;
    float norm_i = m_vectorialPart.x() / norm;
    float norm_j = m_vectorialPart.y() / norm;
    float norm_k = m_vectorialPart.z() / norm;
    m_vectorialPart.setX(norm_i);
    m_vectorialPart.setY(norm_j);
    m_vectorialPart.setZ(norm_k);
}

void Quaternion::print() const
{
    cout << m_scalarPart << " + " << m_vectorialPart.x() << "i + " << m_vectorialPart.y() << "j + " << m_vectorialPart.z() << "k" << endl;
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

//
// Public static methods
//
Quaternion Quaternion::getRotationBetween(const QVector3D& v1, const QVector3D& v2)
{
    float k_cos_theta = QVector3D::dotProduct(v1, v2);
    float k = sqrt( v1.lengthSquared() * v2.lengthSquared() );
    //float k = 1; // Only for unit vectors!
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

Quaternion Quaternion::getRotationBetween(const Point3f &p1, const Point3f &p2, const Point3f &vertex)
{
    QVector3D v1( p1.x() - vertex.x(), p1.y() - vertex.y(), p1.z() - vertex.z() );
    QVector3D v2( p2.x() - vertex.x(), p2.y() - vertex.y(), p2.z() - vertex.z() );
    return Quaternion::getRotationBetween(v1, v2);
}

float Quaternion::getDistanceBetween(const Quaternion &q1, const Quaternion &q2)
{
    return 1 - powf(dotProduct(q1, q2), 2);
}

float Quaternion::dotProduct(const Quaternion &q1, const Quaternion &q2)
{
    return q1.scalar() * q2.scalar() +
           q1.vector().x() * q2.vector().x() +
           q1.vector().y() * q2.vector().y() +
           q1.vector().z() * q2.vector().z();
}

} // End Namespace
