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

    Vector3f* vector = new Vector3f[200];
    qDebug() << "Generating 200 unit vectors";

    for (int i=0; i<200; ++i)
    {
        float pos_x = rand() / (float) RAND_MAX;
        float pos_y = rand() / (float) RAND_MAX;
        float pos_z = rand() / (float) RAND_MAX;

        vector[i].setX(pos_x);
        vector[i].setY(pos_y);
        vector[i].setZ(pos_z);
        vector[i].normalize(); // Get unit vector
    }

    Quaternion* q = new Quaternion[100];
    qDebug() << "Computing quaternion between pairs of vectors";

    for (int i=0; i<100; ++i)
    {
        Vector3f& v1 = vector[i*2];
        Vector3f& v2 = vector[i*2+1];
        q[i] = Quaternion::getRotationBetween(v1, v2);
    }

    qDebug() << "Validating results";

    for (int i=0; i<100; ++i)
    {
        Vector3f& v1 = vector[i*2];
        Vector3f& v2 = vector[i*2+1];
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

bool Quaternion::fuzzyCompare(const QVector3D& v1, const Vector3f &v2)
{
    return qFuzzyCompare(v1.x(), v2.x()) && qFuzzyCompare(v1.y(), v2.y()) && qFuzzyCompare(v1.z(), v2.z());
}

Quaternion::Quaternion()
{
    // Identity quaternion
    m_w = 1.0;
    m_vector = Vector3d(0, 0, 0);
}

Quaternion::Quaternion(double w, double i, double j, double k)
{
    // Identity quaternion
    m_w = w;
    m_vector = Vector3d(i, j, k);
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

void Quaternion::setVector(Vector3d vector)
{
    m_vector = vector;
}

void Quaternion::setVector(double i, double j, double k)
{
    m_vector = Vector3d(i, j, k);
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
        qDebug() << "Quaternion::normalize() -> Not a normalized quaternion (" << this->norm() << ")";
    }
}

void Quaternion::print() const
{
    cout << m_w << " + " << m_vector.x() << "i + " << m_vector.y() << "j + " << m_vector.z() << "k" << endl;
}

QString Quaternion::toString() const
{
    return QString::number(m_w) + " " + QString::number(m_vector.x()) + " " +
            QString::number(m_vector.y()) + " " + QString::number(m_vector.z());
}

double Quaternion::sign(double value) const
{
    double result = 0;

    if (value < 0)
        result = -1;
    else if (value > 0)
        result = 1;

    return result;
}

//
// Public static methods
//
Quaternion Quaternion::getRotationBetween(const Vector3f &v1, const Vector3f &v2)
{
    Vector3d v1d(v1.x(), v1.y(), v1.z());
    Vector3d v2d(v2.x(), v2.y(), v2.z());

    double k_cos_theta = Vector3d::dotProduct(v1d, v2d);
    double k = sqrt( v1d.lengthSquared() * v2d.lengthSquared() );
    //float k = 1; // Only for unit vectors!
    Quaternion result;

    if (k_cos_theta / k != -1)
    {
        double scalarPart = k + k_cos_theta;    
        Vector3d vectorialPart = Vector3d::crossProduct(v1d, v2d); // Not unit vector
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
    Vector3f v1( p1.val(0) - vertex.val(0), p1.val(1) - vertex.val(1), p1.val(2) - vertex.val(2) );
    Vector3f v2( p2.val(0) - vertex.val(0), p2.val(1) - vertex.val(1), p2.val(2) - vertex.val(2) );
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

// Return euler angles from the quaternion as degrees between 0 and 360
Vector3d Quaternion::toEulerAngles() const
{
    double test = m_vector.x() * m_vector.y() + m_vector.z() * m_w;
    double heading = 0.0, attitude = 0.0, bank = 0.0;

    if (test > 0.499) { // singularity at north pole
        heading = 2 * atan2(m_vector.x(), m_w);
        attitude = M_PI/2.0;
        bank = 0.0;
    }
    else if (test < -0.499) { // singularity at south pole
        heading = -2 * atan2(m_vector.x(), m_w);
        attitude = -M_PI/2.0;
        bank = 0.0;
    }
    else {
        double sqx = m_vector.x()*m_vector.x();
        double sqy = m_vector.y()*m_vector.y();
        double sqz = m_vector.z()*m_vector.z();
        //double sqw = m_w*m_w;
        heading = atan2(2*m_vector.y()*m_w-2*m_vector.x()*m_vector.z() , 1 - 2*sqy - 2*sqz);
        attitude = asin(2*test);
        bank = atan2(2*m_vector.x()*m_w-2*m_vector.y()*m_vector.z() , 1 - 2*sqx - 2*sqz);
        // George
        //heading = atan2(2*m_vector.y()*m_w-2*m_vector.x()*m_vector.z(), sqx - sqy - sqz + sqw);
        //attitude = asin(2*test);
        //bank = atan2(2*m_vector.x()*m_w-2*m_vector.y()*m_vector.z() , -sqx + sqy - sqz + sqw);
    }

    //double heading = atan2( 2 * m_vector.y() * m_w - 2 * m_vector.x() * m_vector.z(), 1 - 2 * pow(m_vector.y(),2) - 2 * pow(m_vector.z(),2));
    //double attitude = asin( 2 * m_vector.x() * m_vector.y() + 2 * m_vector.z() * m_w);
    //double bank = atan2( 2 * m_vector.x() * m_w - 2 * m_vector.y() * m_vector.z(), 1 - 2 * pow(m_vector.x(),2) - 2 * pow(m_vector.z(),2));

    double first_angle = heading * 180.0/M_PI;
    double second_angle = attitude*180.0/M_PI;
    double third_angle = bank*180.0/M_PI;

    if (first_angle < 0) first_angle += 360;
    if (second_angle < 0) second_angle += 360;
    if (third_angle < 0) third_angle += 360;

    return Vector3d(first_angle, second_angle, third_angle);
}

} // End Namespace
