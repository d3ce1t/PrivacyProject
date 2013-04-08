#include "Quaternion.h"

namespace dai {

Quaternion::Quaternion()
{
    m_x = m_y = m_z = m_w = 0.0f;
}

Quaternion::Quaternion(float w, float x, float y, float z)
{
    m_x = x;
    m_y = y;
    m_z = z;
    m_w = w;
}

float Quaternion::x() const
{
    return m_x;
}

float Quaternion::y() const
{
    return m_y;
}

float Quaternion::z() const
{
    return m_z;
}

float Quaternion::w() const
{
    return m_w;
}

} // End Namespace
