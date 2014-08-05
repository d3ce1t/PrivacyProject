#ifndef HISTOGRAM2D_H
#define HISTOGRAM2D_H

#include <QHash>
#include <opencv2/opencv.hpp>

namespace dai {

#define DIM_X 0
#define DIM_Y 1
#define DIM_Z 2

/* HistItem */
template <int N>
struct HistItem {
    float point[N];
    int value = 0;
};

typedef HistItem<1> HistItem1D;
typedef HistItem<2> HistItem2D;
typedef HistItem<3> HistItem3D;

/* Histogram */
template <int N>
class Histogram
{
    friend class PrivacyFilter;

public:
    Histogram();
    Histogram(const Histogram& other);
    ~Histogram();
    Histogram& operator=(const Histogram& other);

    //int& at(float y, float x);
    const QList< HistItem<N>* > items() const;
    const HistItem<N>& maxFreqItem() const;
    const HistItem<N>& minFreqItem() const;
    int maxFreq() const;
    int minFreq() const;
    float avgFreq() const;
    int numItems() const;
    int dimensions() const;

private:
    void freeResources();

    QList< HistItem<N>* > m_items;
    HistItem<N> m_min_item;
    HistItem<N> m_max_item;
    int m_min_value;
    int m_max_value;
    float m_avg_value;
};

typedef Histogram<1> Histogram1D;
typedef Histogram<2> Histogram2D;
typedef Histogram<3> Histogram3D;

template <int N>
Histogram<N>::Histogram()
{
    m_min_value = 0;
    m_max_value = 0;
    m_avg_value = 0;
}

template <int N>
Histogram<N>::Histogram(const Histogram<N>& other)
{
   m_min_value = other.m_min_value;
   m_max_value = other.m_max_value;
   m_avg_value = other.m_avg_value;
   m_min_item = other.m_min_item;
   m_max_item = other.m_max_item;

   foreach (HistItem<N>* item, other.m_items) {
       HistItem<N>* itemCopy = new HistItem<N>;
       *itemCopy = *item;
       m_items.append(itemCopy);
   }
}

template <int N>
Histogram<N>& Histogram<N>::operator=(const Histogram<N>& other)
{
    freeResources();

    m_min_value = other.m_min_value;
    m_max_value = other.m_max_value;
    m_avg_value = other.m_avg_value;
    m_min_item = other.m_min_item;
    m_max_item = other.m_max_item;

    foreach (HistItem<N>* item, other.m_items) {
        HistItem<N>* itemCopy = new HistItem<N>;
        *itemCopy = *item;
        m_items.append(itemCopy);
    }

    return *this;
}

template <int N>
Histogram<N>::~Histogram()
{
    freeResources();
}

template <int N>
void Histogram<N>::freeResources()
{
    foreach (HistItem<N>* item, m_items) {
        delete item;
        item = nullptr;
    }

    m_items.clear();
}

/*int& Histogram2D::at(float y, float x)
{
    HistogramItem& item = m_index[y][x];
    return item.value;
}*/

template <int N>
const QList< HistItem<N>* > Histogram<N>::items() const
{
    return m_items; // Implicit sharing
}

template <int N>
const HistItem<N>& Histogram<N>::maxFreqItem() const
{
    return m_max_item;
}

template <int N>
const HistItem<N>& Histogram<N>::minFreqItem() const
{
    return m_min_item;
}

template <int N>
int Histogram<N>::maxFreq() const
{
    return m_max_value;
}

template <int N>
int Histogram<N>::minFreq() const
{
    return m_min_value;
}

template <int N>
float Histogram<N>::avgFreq() const
{
    return m_avg_value;
}

template <int N>
int Histogram<N>::numItems() const
{
    return m_items.size();
}

template <int N>
int Histogram<N>::dimensions() const
{
    return N;
}

} // End Namespace

#endif // HISTOGRAM2D_H
