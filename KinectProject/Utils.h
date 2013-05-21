#ifndef UTILS_H
#define UTILS_H

#include <QList>

namespace dai {

template <class T>
float min(T a, T b) {
    return a < b ? a : b;
}

template <class T>
T max(T a, T b) {
    return a > b ? a : b;
}

template <class T>
T max_element(const T* a, int n)
{
    T max = a[0];

    for (int i=1; i<n; ++i) {
        if (a[i] > max) {
            max = a[i];
        }
    }

    return max;
}

template <class T>
T min_element(const T* a, int n)
{
    T min = a[0];

    for (int i=1; i<n; ++i) {
        if (a[i] < min) {
            min = a[i];
        }
    }

    return min;
}

template <class T>
T avg(const T* a, int n)
{
    T sum = 0;

    for (int i=0; i<n; ++i)
        sum += a[i];

    return n == 0 ? 0 : sum / n;
}

template <class T>T avg(const QList<T>& a)
{
    T sum = 0;
    int n = a.size();

    for (int i=0; i<n; ++i) {
        const T aux = a.at(i);
        sum += aux;
    }

    return n == 0 ? 0 : sum / n;
}

} // End Namespace

#endif // UTILS_H
