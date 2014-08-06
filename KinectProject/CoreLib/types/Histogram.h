#ifndef HISTOGRAM2D_H
#define HISTOGRAM2D_H

#include <QHash>
#include <opencv2/opencv.hpp>
#include <memory>
#include <QString>

namespace dai {

#define DIM_Y 0
#define DIM_X 1
#define DIM_Z 2

/* HistItem */
template <int N>
class HistItem {
public:
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
    QList< HistItem<N>* > m_items;
    HistItem<N> m_min_item;
    HistItem<N> m_max_item;
    int m_min_value;
    int m_max_value;
    float m_avg_value;

    void freeResources()
    {
        foreach (HistItem<N>* item, m_items) {
            delete item;
            item = nullptr;
        }

        m_items.clear();
    }

public:

    Histogram()
    {
        m_min_value = 0;
        m_max_value = 0;
        m_avg_value = 0;
    }

    Histogram(const Histogram& other)
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

    ~Histogram()
    {
        freeResources();
    }

    Histogram& operator=(const Histogram& other)
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

    /*int& at(float y, float x) {
        HistogramItem& item = m_index[y][x];
        return item.value;
    }*/

    const QList< HistItem<N>* > items() const
    {
        return m_items; // Implicit sharing
    }

    const HistItem<N>& maxFreqItem() const
    {
        return m_max_item;
    }

    const HistItem<N>& minFreqItem() const
    {
        return m_min_item;
    }

    int maxFreq() const
    {
        return m_max_value;
    }

    int minFreq() const
    {
        return m_min_value;
    }

    float avgFreq() const
    {
        return m_avg_value;
    }

    int numItems() const
    {
        return m_items.size();
    }

    int dimensions() const
    {
        return N;
    }

    template <class T>
    static shared_ptr<Histogram> create(cv::Mat inputImg, cv::Mat mask)
    {
        Q_ASSERT(inputImg.channels() == N);
        Q_ASSERT( (mask.rows == 0 && mask.cols == 0) || (mask.rows == inputImg.rows && mask.cols == inputImg.cols && mask.depth() == CV_8U) );

        using namespace cv;

        shared_ptr<Histogram<N> > result = make_shared<Histogram<N> >();
        QHash<QString, HistItem<N>*> index;
        int min_value = 999999, max_value = 0, num_pixels = 0;
        HistItem<N> min_item, max_item;

        bool useMask = mask.rows > 0 && mask.cols > 0;

        for (int i=0; i<inputImg.rows; ++i)
        {
            const Vec<T, N>* pPixel = inputImg.ptr<Vec<T, N>>(i);
            const uchar* maskPixel = useMask ? mask.ptr<uchar>(i) : nullptr;

            for (int j=0; j<inputImg.cols; ++j)
            {
                if (useMask && maskPixel[j] <= 0)
                    continue;

                QString hash = QString::number(pPixel[j][0]);
                for (int i=1; i<N; ++i)
                    hash += "#" + QString::number(pPixel[j][i]);

                HistItem<N>*& item = index[hash];

                if (item == nullptr) {
                    item = new HistItem<N>;
                    for (int i=0; i<N; ++i) {
                        item->point[i] = pPixel[j][i];
                    }

                    item->value = 0;
                    result->m_items.append(item);
                }

                item->value++;
                num_pixels++;

                // Min value
                if (item->value < min_value) {
                    min_value = item->value;
                    min_item = *item;
                }
                // Max value
                if (item->value > max_value) {
                    max_value = item->value;
                    max_item = *item;
                }
            }
        }

        result->m_min_value = min_value;
        result->m_min_item = min_item;
        result->m_max_value = max_value;
        result->m_max_item = max_item;
        result->m_avg_value = float(num_pixels) / result->m_items.size();
        return result;
    }

}; // End Histogram

typedef Histogram<1> Histogram1D;
typedef Histogram<2> Histogram2D;
typedef Histogram<3> Histogram3D;

} // End Namespace

#endif // HISTOGRAM2D_H
