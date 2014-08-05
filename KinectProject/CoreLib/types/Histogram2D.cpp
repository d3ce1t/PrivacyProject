#include "Histogram2D.h"

namespace dai {

Histogram2D::Histogram2D()
{
    m_min_value = 0;
    m_max_value = 0;
    m_avg_value = 0;
    m_min_item = nullptr;
    m_max_item = nullptr;
}

Histogram2D::Histogram2D(const Histogram2D& other)
{
   m_min_value = other.m_min_value;
   m_max_value = other.m_max_value;
   m_avg_value = other.m_avg_value;

   foreach (Item* item, other.m_items) {
       Item* itemCopy = new Item;
       *itemCopy = *item;
       m_items.append(itemCopy);

       if (item == other.m_min_item)
           m_min_item = itemCopy;
       else if (item == other.m_max_item)
           m_max_item = itemCopy;
   }
}

Histogram2D& Histogram2D::operator=(const Histogram2D& other)
{
    freeResources();

    m_min_value = other.m_min_value;
    m_max_value = other.m_max_value;
    m_avg_value = other.m_avg_value;

    foreach (Item* item, other.m_items) {
        Item* itemCopy = new Item;
        *itemCopy = *item;
        m_items.append(itemCopy);

        if (item == other.m_min_item)
            m_min_item = itemCopy;
        else if (item == other.m_max_item)
            m_max_item = itemCopy;
    }

    return *this;
}

Histogram2D::~Histogram2D()
{
    freeResources();
}

void Histogram2D::freeResources()
{
    foreach (Item* item, m_items) {
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

const QList<Histogram2D::Item*> Histogram2D::items() const
{
    return m_items; // Implicit sharing
}

const Histogram2D::Item* Histogram2D::maxFreqItem() const
{
    return m_max_item;
}

const Histogram2D::Item* Histogram2D::minFreqItem() const
{
    return m_min_item;
}

int Histogram2D::maxFreq() const
{
    return m_max_value;
}

int Histogram2D::minFreq() const
{
    return m_min_value;
}

float Histogram2D::avgFreq() const
{
    return m_avg_value;
}

int Histogram2D::numItems() const
{
    return m_items.size();
}


} // End Namespace
