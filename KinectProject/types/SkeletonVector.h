#ifndef SKELETONVECTOR_H
#define SKELETONVECTOR_H

#include "SkeletonJoint.h"
#include <QVector3D>

namespace dai {

class SkeletonVector
{
public:

    enum VectorType {
        VECTOR_V1, // CENTER SHOULDER to HEAD
        VECTOR_V2, // CENTER SHOULDER to RIGHT SHOULDER
        VECTOR_V3, // CENTER SHOULDER to LEFT SHOULDER
        VECTOR_V4, // RIGHT SHOULDER to RIGHT ELBOW
        VECTOR_V5, // LEFT SHOULDER to LEFT ELBOW
        VECTOR_V6, // RIGHT ELBOW to RIGHT WRIST
        VECTOR_V7, // LEFT ELBOW to LEFT WRIST
        VECTOR_V8, // RIGHT WRIST to RIGHT HAND
        VECTOR_V9, // LEFT WRIST to LEFT HAND
        VECTOR_V10, // SPINE to CENTER SHOULDER
        VECTOR_V11, // CENTER HIP to SPINE
        VECTOR_V12, // CENTER HIP to RIGHT HIP
        VECTOR_V13, // CENTER HIP to LEFT HIP
        VECTOR_V14, // RIGHT HIP to RIGHT KNEE
        VECTOR_V15, // LEFT HIP to LEFT KNEE
        VECTOR_V16, // RIGHT KNEE to RIGHT ANKLE
        VECTOR_V17, // LEFT KNEE to LEFT ANKLE
        VECTOR_V18, // RIGHT ANKLE to RIGHT FOOT
        VECTOR_V19 // LEFT ANKLE to LEFT FOOT
    };

    SkeletonVector();
    SkeletonVector(SkeletonJoint *p1, SkeletonJoint *p2);
    void setVector(SkeletonJoint *p1, SkeletonJoint *p2);
    const QVector3D& vector() const;

    /**
     * Length of the vector before of being normalized
     * @brief length
     * @return
     */
    float length() const;
    const SkeletonJoint& joint1() const;
    const SkeletonJoint& joint2() const;

private:
    QVector3D m_vector;
    float  m_length; // length before normalise
    SkeletonJoint* m_joint1;
    SkeletonJoint* m_joint2;
};

} // End Namespace

#endif // SKELETONVECTOR_H
