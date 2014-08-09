#include "Quaternion.h"
#include <cmath>
#include <iostream>
#include <ctime>

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
    srand(time(nullptr));

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
    m_w = 1.0f;
    m_vector = Vector3D(0, 0, 0);
}

Quaternion::Quaternion(double w, double i, double j, double k)
    : QObject(0)
{
    // Identity quaternion
    m_w = w;
    m_vector = Vector3D(i, j, k);
}

Quaternion::Quaternion(const Quaternion& other)
    : QObject(0)
{
    m_w = other.scalar();
    m_vector = other.vector();
}

Quaternion& Quaternion::operator=(const Quaternion& other)
{
    this->setScalar(other.scalar());
    this->setVector(other.vector());
    return *this;
}

void Quaternion::setScalar(double value)
{
    m_w = value;
}

void Quaternion::setVector(Vector3D vector)
{
    m_vector = vector;
}

void Quaternion::setVector(double i, double j, double k)
{
    m_vector = Vector3D(i, j, k);
}

double Quaternion::scalar() const
{
    return m_w;
}

Vector3D Quaternion::vector() const
{
    return m_vector;
}

double Quaternion::getAngle() const
{
    // Calculate the rotation angle for this quaternion
    // in order to get it cached.
    // From q2rot function of Octave Quaternions package
    double theta = acos(m_w) * 2;

    if (fabs(theta) > M_PI)
        theta = theta - sign (theta) * M_PI;

    return theta;
}

double Quaternion::norm() const
{
    return sqrt(pow(m_w, 2) +
                pow(m_vector.x(), 2) +
                pow(m_vector.y(), 2) +
                pow(m_vector.z(), 2));
}

void Quaternion::normalize()
{
    double norm = this->norm();
    m_w = m_w / norm;
    double norm_i = m_vector.x() / norm;
    double norm_j = m_vector.y() / norm;
    double norm_k = m_vector.z() / norm;
    m_vector.setX(norm_i);
    m_vector.setY(norm_j);
    m_vector.setZ(norm_k);

    if (fabs(this->norm() - 1) > 1e-12) {
        qDebug() << "Quaternion::normalize() -> Not a normalized quaternion";
    }
}

void Quaternion::print() const
{
    cout << m_w << " + " << m_vector.x() << "i + " << m_vector.y() << "j + " << m_vector.z() << "k" << endl;
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
    Vector3D v1( p1.val(0) - vertex.val(0), p1.val(1) - vertex.val(1), p1.val(2) - vertex.val(2) );
    Vector3D v2( p2.val(0) - vertex.val(0), p2.val(1) - vertex.val(1), p2.val(2) - vertex.val(2) );
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
