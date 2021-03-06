#ifndef POINT_H
#define POINT_H

#include <QtGlobal>
#include <QString>
#include <cmath>
#include <iostream>
#include <climits>

namespace dai {

template <typename T, int N>
class Point
{
    T m_data[N];

    inline static const Point<T,N>& maxPoint()
    {
        static Point<T,N> result;
        static bool firstTime = true;

        if (firstTime) {
            T max_value = std::numeric_limits<T>::max();

            for (int i=0; i<N; ++i)
                result.m_data[i] = max_value;

            firstTime = false;
        }

        return result;
    }

    inline static const Point<T,N>& minPoint()
    {
        static Point<T,N> result;
        static bool firstTime = true;

        if (firstTime) {
            for (int i=0; i<N; ++i)
                result.m_data[i] = 0;

            firstTime = false;
        }

        return result;
    }

public:

    static Point vector(const Point& origin, const Point& dst)
    {
        Point vector;
        for (int i=0; i<N; ++i)
            vector[i] = dst[i] - origin[i];
        return vector;
    }

    static float euclideanDistance(const Point& point1, const Point& point2)
    {
        float sum = 0;

        for (int i=0; i<N; ++i) {
            float diff = point2.val(i) - point1.val(i);
            sum += pow(diff, 2);
        }

        return sqrt(sum);
    }

    static float euclideanDistanceSquared(const Point& point1, const Point& point2)
    {
        float sum = 0;

        for (int i=0; i<N; ++i) {
            float diff = point2.val(i) - point1.val(i);
            sum += pow(diff, 2);
        }

        return sum;
    }

    /**
     * Return the distance between 0 (close) and 1 (far)
     * @brief distanceNorm
     * @param point1
     * @param point2
     * @param option
     * @return
     */
    static float euclideanDistanceNorm(const Point& point1, const Point& point2)
    {
        static float maxDistance = euclideanDistance(Point<T,N>::minPoint(), Point<T,N>::maxPoint());
        float sum = 0;

        for (int i=0; i<N; ++i) {
            float diff = point2.val(i) - point1.val(i);
            sum += pow(diff, 2);
        }

        return sqrt(sum) / maxDistance;
    }

    static Point middlePoint(const Point& p1, const Point& p2)
    {
        Point vector = Point::vector(p1, p2);
        Point medium_point;

        for (int i=0; i<N; ++i)
            medium_point[i] = p1[i] + vector[i]/2;

        return medium_point;
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
        T data[N]={firstValue, static_cast<float>(params)...};
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
        Q_ASSERT(idx < N);
        return m_data[idx];
    }

    inline const T& operator[](int idx) const
    {
        Q_ASSERT(idx < N);
        return const_cast<T&>(m_data[idx]);
    }

    inline const T& val(int idx) const
    {
        Q_ASSERT(idx < N);
        return m_data[idx];
    }

    inline QString toString() const
    {
        QString result = QString::number(m_data[0]);

        for (int i=1; i<N; ++i) {
            result += " " + QString::number(m_data[i]);
        }

        return result;
    }
};

template<typename T>
using Point3D = Point<T, 3>;
using Point3f = Point<float, 3>;
using Point3b = Point<uchar, 3>;

template<typename T>
using Point2D = Point<T, 2>;
using Point2f = Point<float, 2>;
using Point2i = Point<int, 2>;
using Point2b = Point<uchar, 2>;

} // End Namepsace

#endif // POINT_H
