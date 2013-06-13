#ifndef SKELETONQUATERNION_H
#define SKELETONQUATERNION_H

#include <QObject>
#include <QVector3D>
#include "types/Point3f.h"

namespace dai {

class Quaternion : public QObject
{  
    Q_OBJECT
    Q_ENUMS(QuaternionType)

public:
    enum QuaternionType {
        QUATERNION_Q1_V2_V1,
        QUATERNION_Q2_V3_V1,
        QUATERNION_Q3_V2_V4,
        QUATERNION_Q4_V3_V5,
        QUATERNION_Q5_V4_V6,
        QUATERNION_Q6_V5_V7,
        QUATERNION_Q7_V6_V8,
        QUATERNION_Q8_V7_V9,
        QUATERNION_Q9_V11_V10,        
        QUATERNION_Q10_V12_V11,
        QUATERNION_Q11_V13_V11,
        QUATERNION_Q12_V12_V14,
        QUATERNION_Q13_V13_V15,
        QUATERNION_Q14_V14_V16,
        QUATERNION_Q15_V15_V17,
        QUATERNION_Q16_V16_V18,
        QUATERNION_Q17_V17_V19
    };

    /**
     * Get rotation quaternion between two vectors. Vectors not necessarily must be unit vectors
     *
     * @brief getRotationBetween
     * @param v1
     * @param v2
     * @return Return the quaternion that express the rotation over v1 to get the same orientation as v2.
     */
    static Quaternion getRotationBetween(const QVector3D &v1, const QVector3D &v2);

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
    static float getDistanceBetween(const Quaternion &q1, const Quaternion &q2);
    static float dotProduct(const Quaternion &q1, const Quaternion &q2);

    Quaternion();
    Quaternion(const Quaternion& other);
    Quaternion& operator=(const Quaternion& other);

    float scalar() const;
    QVector3D vector() const;
    float getAngle() const;
    float norm() const;
    void print() const;

    void setScalar(float value);
    void setVector(QVector3D vector);
    void setVector(float i, float j, float k);
    void normalize();

private:
    // Private methods
    float sign(float value) const;

    // Private attributes
    float m_theta;
    float m_scalarPart;
    QVector3D m_vectorialPart; // i, j, k or x, y, z
};

} // End Namespace

#endif // SKELETONQUATERNION_H
