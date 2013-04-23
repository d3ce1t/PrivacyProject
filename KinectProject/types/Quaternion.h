#ifndef SKELETONQUATERNION_H
#define SKELETONQUATERNION_H

#include "SkeletonVector.h"
#include <QQuaternion>

namespace dai {

class Quaternion : public QObject, public QQuaternion
{  
    Q_OBJECT
    Q_ENUMS(QuaternionType)

public:
    enum QuaternionType {
        QUATERNION_Q1_V3_V1,
        QUATERNION_Q2_V3_V5,
        QUATERNION_Q3_V5_V7,
        QUATERNION_Q4_V7_V9,
        QUATERNION_Q5_V2_V1,
        QUATERNION_Q6_V2_V4,
        QUATERNION_Q7_V4_V6,
        QUATERNION_Q8_V6_V8,
        QUATERNION_Q9_V11_V10,
        QUATERNION_Q10_V13_V11,
        QUATERNION_Q11_V13_V15,
        QUATERNION_Q12_V15_V17,
        QUATERNION_Q13_V17_V19,
        QUATERNION_Q14_V12_V11,
        QUATERNION_Q15_V12_V14,
        QUATERNION_Q16_V14_V16,
        QUATERNION_Q17_V16_V18
    };

    /**
     * Get rotation between two unit vectors
     *
     * @brief getRotationBetween
     * @param v1
     * @param v2
     * @return
     */
    static Quaternion getRotationBetween(const QVector3D &v1, const QVector3D &v2);

    Quaternion();
    Quaternion(const Quaternion& other);
    virtual ~Quaternion();
    Quaternion& operator=(const Quaternion& other);
};

} // End Namespace

#endif // SKELETONQUATERNION_H
