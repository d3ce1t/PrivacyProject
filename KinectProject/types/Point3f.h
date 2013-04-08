#ifndef POINT3F_H
#define POINT3F_H

namespace dai {

class Point3f
{
public:
    explicit Point3f();
    explicit Point3f(float x, float y, float z);
    Point3f(const Point3f& other);
    void set(float x, float y, float z);
    Point3f& operator=(const Point3f& other);
    bool operator==(const Point3f& other) const;
    bool operator!=(const Point3f& other) const;

    float x() const;
    float y() const;
    float z() const;

private:
    float m_x, m_y, m_z;
};

} // End Namepsace

#endif // POINT3F_H
