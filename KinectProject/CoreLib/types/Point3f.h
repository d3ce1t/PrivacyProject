#ifndef POINT3F_H
#define POINT3F_H

namespace dai {

class Point3f
{
public:

    enum DistanceType {
        DISTANCE_EUCLIDEAN
    };

    static double distance(const Point3f& point1, const Point3f& point2, DistanceType option = DISTANCE_EUCLIDEAN);

    Point3f();
    explicit Point3f(double x, double y, double z);
    Point3f(const Point3f& other);
    void set(double x, double y, double z);
    const double* dataPtr() const;
    Point3f& operator=(const Point3f& other);
    bool operator==(const Point3f& other) const;
    bool operator!=(const Point3f& other) const;

    double x() const;
    double y() const;
    double z() const;

private:    
    static double euclideanDistance(const Point3f& point1, const Point3f& point2);

    double m_pos[3]; // x, y, z
};

} // End Namepsace

#endif // POINT3F_H
