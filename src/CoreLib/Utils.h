#ifndef UTILS_H
#define UTILS_H

#include <limits>
#include <QList>

namespace dai {

template <class T>
inline T min(T a, T b) {
    return a < b ? a : b;
}

template <class T>
inline T max(T a, T b) {
    return a > b ? a : b;
}

template <class T>
T max_element(const T* a, int n)
{
    T max = std::numeric_limits<T>::min();

    for (int i=0; i<n; ++i) {
        if (a[i] > max) {
            max = a[i];
        }
    }

    return max;
}

template <class T>
T min_element(const T* a, int n)
{
    T min = std::numeric_limits<T>::max();

    for (int i=0; i<n; ++i) {
        if (a[i] < min) {
            min = a[i];
        }
    }

    return min;
}

template <class T>
T min_element(const T* a, T min_threshold, int n)
{
    T min = std::numeric_limits<T>::max();

    for (int i=0; i<n; ++i) {
        if (a[i] > min_threshold) {
            if (a[i] < min) {
                min = a[i];
            }
        }
    }

    return min;
}

template <class T>
float avg(const T* a, int n)
{
    T sum = 0;

    for (int i=0; i<n; ++i)
        sum += a[i];

    return n == 0 ? 0 : sum / n;
}

template <class T>
float avg(const QList<T>& a)
{
    T sum = 0;
    int n = a.size();

    for (int i=0; i<n; ++i) {
        const T aux = a.at(i);
        sum += aux;
    }

    return n == 0 ? 0 : sum / n;
}

template <class T>
float normalise(T value, T minValue, T maxValue, float newMin, float newMax)
{
    return (float) ( (value - minValue) * (newMax - newMin) ) / (float) (maxValue - minValue) + newMin;
}

} // End Namespace

#endif // UTILS_H
