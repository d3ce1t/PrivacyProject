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

    Vector3D* vector = new Vector3D[200];
    qDebug() << "Generating 200 unit vectors";

    for (int i=0; i<200; ++i)
    {
        double pos_x = rand() / (double) RAND_MAX;
        double pos_y = rand() / (double) RAND_MAX;
        double pos_z = rand() / (double) RAND_MAX;

        vector[i].setX(pos_x);
        vector[i].setY(pos_y);
        vector[i].setZ(pos_z);
        vector[i].normalize(); // Get unit vector
    }

    Quaternion* q = new Quaternion[100];
    qDebug() << "Computing quaternion between pairs of vectors";

    for (int i=0; i<100; ++i)
    {
        Vector3D& v1 = vector[i*2];
        Vector3D& v2 = vector[i*2+1];
        q[i] = Quaternion::getRotationBetween(v1, v2);
    }

    qDebug() << "Validating results";

    for (int i=0; i<100; ++i)
    {
        Vector3D& v1 = vector[i*2];
        Vector3D& v2 = vector[i*2+1];
        QQuaternion rot(q[i].scalar(), QVector3D(q[i].vector().x(), q[i].vector().y(), q[i].vector().z())); // set up QQuaternion with my computed values
        QVector3D rotateVector(v1.x(), v1.y(), v1.z());
        QVector3D result = rot.rotatedVector(rotateVector); // rotate v1 vector with QQuaternion

        // Now I compare result and v2, they must be the same
        if (fuzzyCompare(result, v2)) {
            qDebug() << "Test " << i+1 << "\t" << "OK";
        } else {
            qDebug() << "Test " << i+1 << "\t" << "Error";
            qDebug() << "Expected" << QVector3D(v2.x(), v2.y(), v2.z());
            qDebug() << "Obtained" << result;
        }
    }

    delete[] vector;
    delete[] q;
}

bool Quaternion::fuzzyCompare(const QVector3D& v1, const Vector3D& v2)
{
    return qFuzzyCompare(v1.x(), (float) v2.x()) && qFuzzyCompare(v1.y(), (float) v2.y()) && qFuzzyCompare(v1.z(), (float) v2.z());
}

Quaternion::Quaternion()
    : QObject(0)
{
    // Identity quaternion
    m_scalarPart = 1.0f;
    m_vectorialPart = Vector3D(0, 0, 0);
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

void Quaternion::setScalar(double value)
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

void Quaternion::setVector(Vector3D vector)
{
    m_vectorialPart = vector;
}

void Quaternion::setVector(double i, double j, double k)
{
    m_vectorialPart = Vector3D(i, j, k);
}

double Quaternion::scalar() const
{
    return m_scalarPart;
}

Vector3D Quaternion::vector() const
{
    return m_vectorialPart;
}

double Quaternion::getAngle() const
{
    return m_theta;
}

double Quaternion::norm() const
{
    return sqrt(pow(m_scalarPart, 2) +
                pow(m_vectorialPart.x(), 2) +
                pow(m_vectorialPart.y(), 2) +
                pow(m_vectorialPart.z(), 2));
}

void Quaternion::normalize()
{
    double norm = this->norm();
    m_scalarPart = m_scalarPart / norm;
    double norm_i = m_vectorialPart.x() / norm;
    double norm_j = m_vectorialPart.y() / norm;
    double norm_k = m_vectorialPart.z() / norm;
    m_vectorialPart.setX(norm_i);
    m_vectorialPart.setY(norm_j);
    m_vectorialPart.setZ(norm_k);

    if (fabs(this->norm() - 1) > 1e-12) {
        qDebug() << "Quaternion::normalize() -> Not a normalized quaternion";
    }
}

void Quaternion::print() const
{
    cout << m_scalarPart << " + " << m_vectorialPart.x() << "i + " << m_vectorialPart.y() << "j + " << m_vectorialPart.z() << "k" << endl;
}

double Quaternion::sign(double value) const
{
    double result = 0;

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
Quaternion Quaternion::getRotationBetween(const Vector3D& v1, const Vector3D& v2)
{
    double k_cos_theta = Vector3D::dotProduct(v1, v2);
    double k = sqrt( v1.lengthSquared() * v2.lengthSquared() );
    //float k = 1; // Only for unit vectors!
    Quaternion result;

    if (k_cos_theta / k != -1)
    {
        double scalarPart = k + k_cos_theta;
        Vector3D vectorialPart = Vector3D::crossProduct(v1, v2); // Not unit vector
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
    Vector3D v1( p1.x() - vertex.x(), p1.y() - vertex.y(), p1.z() - vertex.z() );
    Vector3D v2( p2.x() - vertex.x(), p2.y() - vertex.y(), p2.z() - vertex.z() );
    return Quaternion::getRotationBetween(v1, v2);
}

double Quaternion::getDistanceBetween(const Quaternion &q1, const Quaternion &q2)
{
    double dotProduct = Quaternion::dotProduct(q1, q2);
    double result = 1 - pow(dotProduct, 2);

    if (result < 0) {
        qDebug() << "Quaternion distance cannot be lower than 0";
    } else if (result > 1) {
        qDebug() << "Quaternion distance cannot be higher than 1";
    }

    return result;
}

double Quaternion::dotProduct(const Quaternion &q1, const Quaternion &q2)
{
    return q1.scalar() * q2.scalar() +
           q1.vector().x() * q2.vector().x() +
           q1.vector().y() * q2.vector().y() +
           q1.vector().z() * q2.vector().z();
}

} // End Namespace
