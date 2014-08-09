#ifndef POINT_H
#define POINT_H

#include <QtGlobal>
#include <cmath>
#include <iostream>

namespace dai {

template <typename T, int N>
class Point
{
    static float euclideanDistance(const Point& point1, const Point& point2)
    {
        float sum = 0;

        for (int i=0; i<N; ++i) {
            float diff = point2.val(i) - point1.val(i);
            sum += pow(diff, 2);
        }

        return sqrt(sum);
    }

    T m_data[N];

public:

    enum DistanceType {
        DISTANCE_EUCLIDEAN
    };

    static float distance(const Point& point1, const Point& point2, DistanceType option = DISTANCE_EUCLIDEAN)
    {
        float result = 1.0e6;

        if (option == DISTANCE_EUCLIDEAN) {
            result = euclideanDistance(point1, point2);
        }

        return result;
    }

    Point()
    {
        for (int i=0; i<N; ++i) {
            m_data[i] = 0;
        }
    }


    template <typename...Args>
    explicit Point(T firstValue, Args...params)
    {
        Q_ASSERT(sizeof...(params) == N-1);
        T data[N]={firstValue, params...};
        memcpy(m_data, data, N * sizeof(T));
    }

    Point(const Point& other)
    {
        memcpy(m_data, other.m_data, N * sizeof(T));
    }

    Point& operator=(const Point& other)
    {
        memcpy(m_data, other.m_data, N * sizeof(T));
        return *this;
    }

    const T* dataPtr() const
    {
        return m_data;
    }

    bool operator==(const Point& other) const
    {
        bool equal = true;
        int i = 0;

        while (i<N && equal) {
            equal = (m_data[i] == other.m_data[i]);
            i++;
        }

        return equal;
    }

    bool operator!=(const Point& other) const
    {
        return !operator==(other);
    }

    inline T& operator[](int idx)
    {
        Q_ASSERT(pos < N);
        return m_data[idx];
    }

    inline const T& operator[](int idx) const
    {
        Q_ASSERT(pos < N);
        return const_cast<T&>(m_data[idx]);
    }

    inline const T& val(int pos) const
    {
        Q_ASSERT(pos < N);
        return m_data[pos];
    }
};

using Point3f = Point<float, 3>;
using Point2f = Point<float, 2>;

} // End Namepsace

#endif // POINT_H
