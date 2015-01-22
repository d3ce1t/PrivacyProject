#ifndef QUATERNION_H
#define QUATERNION_H

#include <QObject>
#include "Vector3D.h"
#include "Point.h"

namespace dai {

class Quaternion : public QObject
{  
    Q_OBJECT
    Q_ENUMS(QuaternionType)

public:
    enum QuaternionType {
        QUATERNION_Q1 = 0,
        QUATERNION_Q2,
        QUATERNION_Q3,
        QUATERNION_Q4,
        QUATERNION_Q5,
        QUATERNION_Q6,
        QUATERNION_Q7,
        QUATERNION_Q8,
        QUATERNION_Q9,
        QUATERNION_Q10,
        QUATERNION_Q11,
        QUATERNION_Q12,
        QUATERNION_Q13,
        QUATERNION_Q14,
        QUATERNION_Q15,
        QUATERNION_Q16,
        QUATERNION_Q17,
        QUATERNION_Q18,
        QUATERNION_Q19,
        QUATERNION_Q20,
        QUATERNION_Q21,
        QUATERNION_Q22
    };

    /**
     * Test the quaternion class method getRotationBetween. In that aspect, it creates 200 vectors in a
     * random way and then computes the quaternion for each pair of vectors. Finally, it uses QQuaternion class
     * to rotate the first vector of the pair. Comparing the obtained result with the second vector of the pair
     * it is determined the right behaviour of this class.
     *
     * @brief test
     */
    static void test();
    static bool fuzzyCompare(const QVector3D& v1, const Vector3f &v2);

    /**
     * Get rotation quaternion between two vectors. Vectors not necessarily must be unit vectors
     *
     * @brief getRotationBetween
     * @param v1
     * @param v2
     * @return Return the quaternion that express the rotation over v1 to get the same orientation as v2.
     */
    static Quaternion getRotationBetween(const Vector3f &v1, const Vector3f &v2);

    /**
     * Get rotation quaternion between points p1 and p2, where both points share de same vertex. In other words,
     * Compute the quaternion that express the rotation over vector vertex to p1, to get the same orientation as
     * vector vertex to p2.
     *
     * @brief getRotationBetween
     * @param p1
     * @param p2
     * @param vertex
     * @return Return the indicated quaternion
     */
    static Quaternion getRotationBetween(const Point3f &p1, const Point3f &p2, const Point3f &vertex);

    /**
     * Get distance between two quaternions.
     *
     * @brief getDistanceBetween
     * @param q1
     * @param q2
     * @return Returns 0 when same orientation and 1 when 180º apart.
     */
    static double getDistanceBetween(const Quaternion &q1, const Quaternion &q2);
    static double dotProduct(const Quaternion &q1, const Quaternion &q2);

    Quaternion();
    Quaternion(float w, float i, float j, float k);
    Quaternion(const Quaternion& other);
    Quaternion& operator=(const Quaternion& other);
    void setScalar(float value);
    void setVector(Vector3f vector);
    void setVector(float i, float j, float k);
    float scalar() const {return m_w;}
    Vector3f vector() const {return m_vector;}
    double getAngle() const;
    double norm() const;
    float w() const {return m_w;}
    float x() const {return m_vector.x();}
    float y() const {return m_vector.y();}
    float z() const {return m_vector.z();}
    void normalize();
    void print() const;
    QString toString() const;
    Vector3d toEulerAngles() const;

protected:
    double sign(double value) const;

private:
    float m_w;
    Vector3f m_vector; // i, j, k or x, y, z
};

} // End Namespace

#endif // QUATERNION_H
