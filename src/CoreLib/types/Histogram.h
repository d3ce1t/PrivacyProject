#ifndef HISTOGRAM2D_H
#define HISTOGRAM2D_H

#include <QMap>
#include <QMultiMap>
#include "types/Point.h"
#include <opencv2/opencv.hpp>
#include <memory>
#include <cmath>
#include <climits>
#include <QString>
#include <QHash>
#include <QObject>
#include "Utils.h"

namespace dai {

/* Hash item functions */
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
    return point[0]*65536 + point[1]*256 + point[2];
}

template<>
inline uint hashItem(const cv::Vec<uchar,2>& point)
{
    return point[0]*256 + point[1];
}

template<>
inline uint hashItem(const cv::Vec<ushort,1>& point)
{
    return point[0];
}

template<>
inline uint hashItem(const cv::Vec<uchar,1>& point)
{
    return point[0];
}

/* HistBin */
template <typename T, int N>
class HistBin {
public:
    Point<T, N> point;
    int value = 0;
    double dist = 0;
    uint key = 0;

    bool operator==(const HistBin& other) const
    {
        return point == other.point && value == other.value && dist == other.dist;
    }

    inline static float distanceColor(const HistBin<T,N>& item1, const HistBin<T,N>& item2)
    {
        return Point<T,N>::euclideanDistanceNorm(item1.point, item2.point);
    }

    inline static float distanceSize(const HistBin<T,N>& item1, const HistBin<T,N>& item2)
    {
        double higher = dai::max<double>(item1.dist, item2.dist);
        double lower = dai::min<double>(item1.dist, item2.dist);
        return 1.0 - (lower / higher);
    }

    inline static float distance(const HistBin<T,N>& item1, const HistBin<T,N>& item2)
    {
        float color_distance = distanceColor(item1, item2);
        float size_distance = distanceSize(item1, item2);
        return 0.9*color_distance + 0.1*size_distance;
    }
};

/* Histogram */
template <class T, int N>
class Histogram
{
    QMap<uint, HistBin<T,N>> m_matrix; // It's like a sparse matrix but with a trick
    HistBin<T,N> m_min_item;
    HistBin<T,N> m_max_item;
    int m_min_freq;
    int m_max_freq;
    float m_avg_freq;
    int m_accumulated_freq;
    int m_min_range;
    int m_max_range;

    inline void computeStats()
    {
        int min_freq = std::numeric_limits<int>::max(), max_freq = 0;
        HistBin<T,N> min_item, max_item;

        for (auto it = m_matrix.begin(); it != m_matrix.end(); ++it) {

            // Dist normalised
            (*it).dist = double((*it).value) / double(m_accumulated_freq);

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
        m_avg_freq = float(m_accumulated_freq) / m_matrix.size(); // Avg
    }

public:

    Histogram()
    {
        m_min_freq = 0;
        m_max_freq = 0;
        m_avg_freq = 0;
        m_accumulated_freq = 0;
        m_min_range = 0;
        m_max_range = 0;
    }

    /*Histogram(const Histogram& other)
    {
        m_min_freq = other.m_min_freq;
        m_max_freq = other.m_max_freq;
        m_avg_freq = other.m_avg_freq;
        m_accumulated_freq = other.m_accumulated_freq;
        m_min_item = other.m_min_item;
        m_max_item = other.m_max_item;
        m_min_range = other.m_min_range;
        m_max_range = other.m_max_range;
        m_matrix = other.m_matrix; // Implicit sharing
    }*/

    /*Histogram& operator=(const Histogram& other)
    {
        m_min_freq = other.m_min_freq;
        m_max_freq = other.m_max_freq;
        m_avg_freq = other.m_avg_freq;
        m_accumulated_freq = other.m_accumulated_freq;
        m_min_item = other.m_min_item;
        m_max_item = other.m_max_item;
        m_min_range = other.m_min_range;
        m_max_range = other.m_max_range;
        m_matrix = other.m_matrix; // Implicit sharing
        return *this;
    }*/

    int sizeInBytes() const
    {
        return sizeof(Histogram<T,N>) + m_matrix.size() * sizeof(HistBin<T,N>);
    }

    Histogram<T,N>& operator+=(const Histogram<T,N>& right)
    {
        // Add the two histograms
        for (auto it = right.m_matrix.constBegin(); it != right.m_matrix.constEnd(); ++it)
        {
            const HistBin<T,N>& right_bin = *it;
            HistBin<T,N>& left_bin = this->m_matrix[it.key()]; // Get an alias (create item if no exist)

            // Didn't exist
            if (left_bin.value == 0) {
                left_bin = right_bin; // Copy
            }
            else {
                left_bin.value += right_bin.value;
                //left_bin.point = right_bin.point // I assume left_item.point is alright
            }
        }

        // Add accumulated freq
        this->m_accumulated_freq += right.m_accumulated_freq;
        this->m_min_range = right.m_min_range;
        this->m_max_range = right.m_max_range;
        this->computeStats();

        return *this;
    }

    bool operator==(const Histogram<T,N>& other) const
    {
        return m_matrix == other.m_matrix;
    }

    inline const HistBin<T,N>& maxFreqItem() const
    {
        return m_max_item;
    }

    inline const HistBin<T,N>& minFreqItem() const
    {
        return m_min_item;
    }

    inline int maxFreq() const
    {
        return m_max_freq;
    }

    inline int minFreq() const
    {
        return m_min_freq;
    }

    inline float avgFreq() const
    {
        return m_avg_freq;
    }

    inline int numItems() const
    {
        return m_matrix.size();
    }

    inline int dimensions() const
    {
        return N;
    }

    inline int minRange() const
    {
        return m_min_range;
    }

    inline int maxRange() const
    {
        return m_max_range;
    }

    /**
     * Get n bins from the histogram sorted from lower key to higher key
     */
    QList<const HistBin<T,N>*> bins(int n = 0) const
    {
        QList<const HistBin<T,N>*> list;

        n = n <= 0 ? m_matrix.size() : n;

        auto it = m_matrix.constBegin();
        int i=0;

        while (it != m_matrix.constEnd() && i < n) {
            const HistBin<T,N>& item = *it;
            list.append(&item);
            ++it;
            ++i;
        }

        return list;
    }

    /**
     * Get the n bins of higher frequency
     */
    QList<const HistBin<T,N>*> higherFreqBins(int n = 0) const
    {
        QList< const HistBin<T,N>*> list;
        QMultiMap<int, const HistBin<T,N>*> index; // QMap is ordered from lower to higher freq

        for (auto it = m_matrix.constBegin(); it != m_matrix.constEnd(); ++it ) {
            const HistBin<T,N>& item = *it;
            index.insert(item.value, &item);            
        }

        // Invert it
        n = n <= 0 ? m_matrix.size() : n;
        auto it_list = index.constEnd();
        int i=0;

        while (it_list != index.constBegin() && i < n) {
            --it_list;
            list.append(*it_list);
            ++i;
        }

        return list;
    }

    double distance(const Histogram<T,N>& other) const
    {
        return Histogram<T,N>::intersection(*this, other);
    }

    float entropy(int n_often_items = 0) const
    {
        float entropy = 0;

        QList<const HistBin<T,N>*> items = higherFreqBins(n_often_items);

        for (auto it = items.constBegin(); it != items.constEnd(); ++it)
        {
            const HistBin<T,N>* item = *it;
            entropy += (item->dist * std::log2(item->dist));
        }

        return -entropy;
    }

    float distanceIntraClass(int n_often_items = 0) const
    {
        QList<const HistBin<T,N>*> items = higherFreqBins(n_often_items);
        float distance = 0;

        for (auto it1 = items.constBegin(); it1 != items.constEnd(); ++it1)
        {
            for (auto it2 = items.constBegin(); it2 != items.constEnd(); ++it2)
            {
                distance += HistBin<T,N>::distanceColor( *(*it1), *(*it2) );
            }
        }

        return distance / (items.size() * items.size());
    }

    const std::shared_ptr<Histogram<T,N> > subHistogram(int n_often_items)
    {
        std::shared_ptr<Histogram<T,N>> result = std::make_shared<Histogram<T,N>>();

        QList<const HistBin<T,N>*> items = higherFreqBins(n_often_items);

        for (auto it = items.constBegin(); it != items.constEnd(); ++it)
        {
            auto item = *it;
            cv::Vec<T,N> point;

            for (int i=0; i<N; ++i) {
                point[i] = item->point[i];
            }

            int hash = hashItem<T,N>(point);
            HistBin<T,N>& dstItem = result->m_matrix[hash];
            dstItem.point = item->point;
            dstItem.value = item->value;
            result->m_accumulated_freq += item->value;
        }

        result->computeStats();
        return result;
    }

    const static std::shared_ptr<Histogram<T,N> > create(cv::Mat inputImg, std::vector<int> ranges, cv::Mat mask = cv::Mat(), uchar value = 1)
    {
        Q_ASSERT(inputImg.channels() == N);
        Q_ASSERT( (mask.rows == 0 && mask.cols == 0) || (mask.rows == inputImg.rows && mask.cols == inputImg.cols && mask.depth() == CV_8U) );

        using namespace cv;

        std::shared_ptr<Histogram<T,N>> result = std::make_shared<Histogram<T,N>>();
        bool useMask = mask.rows > 0 && mask.cols > 0;

        // Compute Histogram
        for (int i=0; i<inputImg.rows; ++i)
        {
            const Vec<T,N>* pPixel = inputImg.ptr<Vec<T,N>>(i);
            const uchar* maskPixel = useMask ? mask.ptr<uchar>(i) : nullptr;

            for (int j=0; j<inputImg.cols; ++j)
            {
                if (useMask && maskPixel[j] != value)
                    continue;

                uint hash = hashItem<T,N>(pPixel[j]);
                HistBin<T,N>& item = result->m_matrix[hash];

                if (item.value == 0) {
                    for (int i=0; i<N; ++i) {
                        item.point[i] = pPixel[j][i];
                    }
                    item.key = hash;
                }

                item.value++;
                result->m_accumulated_freq++;
            }
        }

        // Compute stats
        result->computeStats();
        result->m_min_range = ranges[0];
        result->m_max_range = ranges[1];
        return result;
    }


    /**
     * Compute the distance as the average distance element by element.
     */
    static float distance1(const Histogram<T,N>& hist1, const Histogram<T,N>& hist2, int n_elements = 0)
    {
        int max_elements = dai::min<int>(hist1.numItems(), hist2.numItems());
        Q_ASSERT(n_elements <= max_elements);

        if (n_elements <= 0)
            n_elements = max_elements;

        auto h1_items = hist1.higherFreqBins(n_elements);
        auto h2_items = hist2.higherFreqBins(n_elements);
        auto it_h1 = h1_items.constBegin();
        auto it_h2 = h2_items.constBegin();

        float distance = 0;

        while (it_h1 != h1_items.constEnd() && it_h2 != h2_items.constEnd())
        {
            const HistBin<T,N>* item1 = *it_h1;
            const HistBin<T,N>* item2 = *it_h2;
            distance += HistBin<T,N>::distanceColor(*item1, *item2);

            // Next items
            ++it_h1;
            ++it_h2;
        }

        return distance / n_elements;
    }

    /**
     * Compute the distance as the average distance element by element, but also consider the size of each
     * item. So, final distance is weighted as 0.6*color and 0.4*size
     * @brief distance
     * @param hist1
     * @param hist2
     * @param n_elements
     * @return
     */
    static float distance2(const Histogram<T,N>& hist1, const Histogram<T,N>& hist2, int n_elements = 0)
    {
        int max_elements = dai::min<int>(hist1.numItems(), hist2.numItems());
        Q_ASSERT(n_elements <= max_elements);

        if (n_elements <= 0)
            n_elements = max_elements;

        auto h1_items = hist1.higherFreqBins(n_elements);
        auto h2_items = hist2.higherFreqBins(n_elements);
        auto it_h1 = h1_items.constBegin();
        auto it_h2 = h2_items.constBegin();

        float distance = 0;

        while (it_h1 != h1_items.constEnd() && it_h2 != h2_items.constEnd())
        {
            const HistBin<T,N>* item1 = *it_h1;
            const HistBin<T,N>* item2 = *it_h2;
            distance += HistBin<T,N>::distance(*item1, *item2);

            // Next items
            ++it_h1;
            ++it_h2;
        }

        return distance / n_elements;
    }

    // All by All
    static float distance3(const Histogram<T,N>& hist1, const Histogram<T,N>& hist2, int n_elements = 0)
    {
        int max_elements = dai::min<int>(hist1.numItems(), hist2.numItems());
        Q_ASSERT(n_elements <= max_elements);

        if (n_elements <= 0)
            n_elements = max_elements;

        auto h1_items = hist1.higherFreqBins(n_elements);
        auto h2_items = hist2.higherFreqBins(n_elements);

        float distance = 0;

        for (auto it1 = h1_items.constBegin(); it1 != h1_items.constEnd(); ++it1)
        {
            for (auto it2 = h2_items.constBegin(); it2 != h2_items.constEnd(); ++it2)
            {
                const HistBin<T,N>* item1 = *it1;
                const HistBin<T,N>* item2 = *it2;
                distance += HistBin<T,N>::distanceColor(*item1, *item2);
            }
        }

        return distance / (h1_items.size() * h2_items.size());
    }

    /**
     * Intersection similarity measure between two histograms
     */
    static double intersection(const Histogram<T,N>& hist1, const Histogram<T,N>& hist2)
    {
        double distance = 0;

        for (auto it = hist1.m_matrix.constBegin(); it != hist1.m_matrix.constEnd(); ++it)
        {
            const HistBin<T,N>& item1 = it.value();

            if (hist2.m_matrix.contains(it.key())) {
                const HistBin<T,N>& item2 = hist2.m_matrix[it.key()];
                distance += dai::min<double>(item1.dist, item2.dist);
            }
        }

        distance = 1.0 - distance;
        return distance;
    }

    void print(int n_elems)
    {
        // Show info of the histogram
        int i=0;
        foreach (auto item, this->higherFreqBins(n_elems)) {
            std::cout << i++ << "(" << item->point.toString().toStdString() << ")" << item->value << item->dist << std::endl;
        }
    }

}; // End Histogram

// Histogram Definitions
template<typename T>
using Histogram1D = Histogram<T, 1>;

template<typename T>
using Histogram2D = Histogram<T, 2>;

template<typename T>
using Histogram3D = Histogram<T, 3>;

using Histogram3f = Histogram<float, 3>;
using Histogram3c = Histogram<uchar, 3>;
using Histogram2c = Histogram<uchar, 2>;
using Histogram2s = Histogram<ushort, 2>;
using Histogram1c = Histogram<uchar, 1>;
using Histogram1s = Histogram<ushort, 1>;

// HistItem definitions
template<typename T>
using HistBin1D = HistBin<T, 1>;

template<typename T>
using HistBin2D = HistBin<T, 2>;

template<typename T>
using HistBin3D = HistBin<T, 3>;

using HistBin3f = HistBin<float, 3>;
using HistBin3c = HistBin<uchar, 3>;
using HistBin2c = HistBin<uchar, 2>;
using HistBin2s = HistBin<ushort, 2>;
using HistBin1c = HistBin<uchar, 1>;
using HistBin1s = HistBin<ushort, 1>;

} // End Namespace

#endif // HISTOGRAM2D_H
