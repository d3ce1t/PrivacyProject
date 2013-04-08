#ifndef QUATERNION_H
#define QUATERNION_H

namespace dai {

class Quaternion
{
public:
    Quaternion();
    Quaternion(float w, float x, float y, float z);

    float x() const;
    float y() const;
    float z() const;
    float w() const;

private:
    float m_x, m_y, m_z, m_w;
};

} // End Namespace

#endif // QUATERNION_H
