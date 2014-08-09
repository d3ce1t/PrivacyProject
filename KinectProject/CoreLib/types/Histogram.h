#ifndef HISTOGRAM2D_H
#define HISTOGRAM2D_H

#include <QHash>
#include <QMultiMap>
#include "types/Point.h"
#include <opencv2/opencv.hpp>
#include <memory>
#include <QString>
#include <QObject>
#include "Utils.h"

namespace dai {

#define DIM_Y 0
#define DIM_X 1
#define DIM_Z 2

template<class T, int N>
inline uint hashItem(const cv::Vec<T,N>& point)
{
    QString key = QString::number(point[0]);
    for (int i=1; i<N; ++i)
        key += "#" + QString::number(point[i]);

    return qHash(key);
}

template<>
inline uint hashItem(const cv::Vec<uchar,3>& point)
{
    return (point[0]*256 + point[1])*256 + point[2];
}


/* HistItem */
template <typename T, int N>
class HistItem {
public:
    Point<T, N> point;
    int value = 0;
    float dist = 0;
};

/* Histogram */
template <class T, int N>
class Histogram
{
    QHash<uint, HistItem<T,N>> m_index;
    HistItem<T,N> m_min_item;
    HistItem<T,N> m_max_item;
    int m_min_freq;
    int m_max_freq;
    float m_avg_value;
    int m_accumulated_freq;

    inline void computeStats()
    {
        int min_freq = 999999, max_freq = 0;
        HistItem<T,N> min_item, max_item;

        for (auto it = m_index.begin(); it != m_index.end(); ++it) {

            // Dist normalised
            (*it).dist = float((*it).value) / float(m_accumulated_freq);

            // Min
            if ((*it).value < min_freq) {
                min_freq = (*it).value;
                min_item = *it;
            }

            // Max
            if ((*it).value > max_freq) {
                max_freq = (*it).value;
                max_item = *it;
            }
        }

        m_min_freq = min_freq;
        m_min_item = min_item;
        m_max_freq = max_freq;
        m_max_item = max_item;
        m_avg_value = float(m_accumulated_freq) / m_index.size(); // Avg
    }

public:

    Histogram()
    {
        m_min_freq = 0;
        m_max_freq = 0;
        m_avg_value = 0;
        m_accumulated_freq = 0;
    }

    Histogram(const Histogram& other)
    {
        m_min_freq = other.m_min_freq;
        m_max_freq = other.m_max_freq;
        m_avg_value = other.m_avg_value;
        m_accumulated_freq = other.m_accumulated_freq;
        m_min_item = other.m_min_item;
        m_max_item = other.m_max_item;
        m_index = other.m_index; // Implicit sharing
    }

    Histogram& operator=(const Histogram& other)
    {
        m_min_freq = other.m_min_freq;
        m_max_freq = other.m_max_freq;
        m_avg_value = other.m_avg_value;
        m_accumulated_freq = other.m_accumulated_freq;
        m_min_item = other.m_min_item;
        m_max_item = other.m_max_item;
        m_index = other.m_index; // Implicit sharing
        return *this;
    }

    Histogram<T,N>& operator+=(const Histogram<T,N>& right)
    {
        // Add the two histograms
        for (auto it = right.m_index.constBegin(); it != right.m_index.constEnd(); ++it)
        {
            const HistItem<T,N>& right_item = *it;
            HistItem<T,N>& left_item = this->m_index[it.key()]; // Get an alias (create item if no exist)

            // Didn't exist
            if (left_item.value == 0) {
                left_item = right_item; // Copy
            }
            else {
                // I assume left_item.point is alright
                left_item.value += right_item.value;
            }
        }

        // Add accumulated freq
        this->m_accumulated_freq += right.m_accumulated_freq;        
        this->computeStats();

        return *this;
    }

    const HistItem<T,N>& maxFreqItem() const
    {
        return m_max_item;
    }

    const HistItem<T,N>& minFreqItem() const
    {
        return m_min_item;
    }

    int maxFreq() const
    {
        return m_max_freq;
    }

    int minFreq() const
    {
        return m_min_freq;
    }

    float avgFreq() const
    {
        return m_avg_value;
    }

    int numItems() const
    {
        return m_index.size();
    }

    int dimensions() const
    {
        return N;
    }

    QList<const HistItem<T,N>*> items(int n_elements = 0) const
    {
        QList<const HistItem<T,N>*> list;

        n_elements = n_elements <= 0 ? m_index.size() : n_elements;

        auto it = m_index.constBegin();
        int i=0;

        while (it != m_index.constEnd() && i < n_elements) {
            const HistItem<T,N>& item = *it;
            list.append(&item);
            ++it;
            ++i;
        }

        return list;
    }

    QList<const HistItem<T,N>*> sortedItems(int n_elements = 0) const
    {
        QList< const HistItem<T,N>*> list;
        QMultiMap< int, const HistItem<T,N>*> index; // QMap is ordered from lower to higher freq

        for (auto it = m_index.constBegin(); it != m_index.constEnd(); ++it ) {
            const HistItem<T,N>& item = *it;
            index.insert(item.value, &item);            
        }

        // Invert it
        n_elements = n_elements <= 0 ? m_index.size() : n_elements;
        auto it_list = index.constEnd();
        int i=0;

        while (it_list != index.constBegin() && i < n_elements) {
            --it_list;
            list.append(*it_list);
            ++i;
        }

        return list;
    }

    const static std::shared_ptr<Histogram<T,N> > create(cv::Mat inputImg, cv::Mat mask)
    {
        Q_ASSERT(inputImg.channels() == N);
        Q_ASSERT( (mask.rows == 0 && mask.cols == 0) || (mask.rows == inputImg.rows && mask.cols == inputImg.cols && mask.depth() == CV_8U) );

        using namespace cv;

        std::shared_ptr<Histogram<T,N>> result = make_shared<Histogram<T,N>>();
        bool useMask = mask.rows > 0 && mask.cols > 0;

        // Compute Histogram
        for (int i=0; i<inputImg.rows; ++i)
        {
            const Vec<T,N>* pPixel = inputImg.ptr<Vec<T,N>>(i);
            const uchar* maskPixel = useMask ? mask.ptr<uchar>(i) : nullptr;

            for (int j=0; j<inputImg.cols; ++j)
            {
                if (useMask && maskPixel[j] <= 0)
                    continue;

                int hash = hashItem<T,N>(pPixel[j]);
                HistItem<T,N>& item = result->m_index[hash];

                if (item.value == 0) {
                    for (int i=0; i<N; ++i) {
                        item.point[i] = pPixel[j][i];
                    }
                }

                item.value++;
                result->m_accumulated_freq++;
            }
        }

        result->computeStats();
        return result;
    }

}; // End Histogram

// Definitions
template<typename T>
using Histogram1D = Histogram<T, 1>;

template<typename T>
using Histogram2D = Histogram<T, 2>;

template<typename T>
using Histogram3D = Histogram<T, 3>;

template<typename T>
using HistItem1D = HistItem<T, 1>;

template<typename T>
using HistItem2D = HistItem<T, 2>;

template<typename T>
using HistItem3D = HistItem<T, 3>;

} // End Namespace

#endif // HISTOGRAM2D_H
