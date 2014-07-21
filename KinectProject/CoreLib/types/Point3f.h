#ifndef POINT3F_H
#define POINT3F_H

namespace dai {

class Point3f
{
public:

    enum DistanceType {
        DISTANCE_EUCLIDEAN
    };

    static float distance(const Point3f& point1, const Point3f& point2, DistanceType option = DISTANCE_EUCLIDEAN);

    Point3f();
    explicit Point3f(float x, float y, float z);
    Point3f(const Point3f& other);
    const float *dataPtr() const;
    Point3f& operator=(const Point3f& other);
    bool operator==(const Point3f& other) const;
    bool operator!=(const Point3f& other) const;

    float x() const;
    float y() const;
    float z() const;

private:    
    static float euclideanDistance(const Point3f& point1, const Point3f& point2);
    void set(float x, float y, float z);

    float m_pos[3]; // x, y, z
};

} // End Namepsace

#endif // POINT3F_H
