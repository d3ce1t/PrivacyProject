#ifndef HISTOGRAM2D_H
#define HISTOGRAM2D_H

#include <QHash>
#include <opencv2/opencv.hpp>

namespace dai {

class Histogram2D
{
    friend class PrivacyFilter;

public:

    struct Item {
        float x;
        float y;
        int value = 0;
    };

    Histogram2D();
    Histogram2D(const Histogram2D& other);
    ~Histogram2D();
    Histogram2D& operator=(const Histogram2D& other);

    //int& at(float y, float x);
    const QList<Item*> items() const;
    const Item* maxFreqItem() const;
    const Item* minFreqItem() const;
    int maxFreq() const;
    int minFreq() const;
    float avgFreq() const;
    int numItems() const;

private:
    void freeResources();

    QList<Item*> m_items;
    Item* m_min_item;
    Item* m_max_item;
    int m_min_value;
    int m_max_value;
    float m_avg_value;
};

} // End Namespace

#endif // HISTOGRAM2D_H
