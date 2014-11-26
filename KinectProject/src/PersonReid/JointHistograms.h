#ifndef JOINT_HISTOGRAMS_H
#define JOINT_HISTOGRAMS_H

#include <memory>
#include "dataset/InstanceInfo.h"
#include "types/Histogram.h"
#include <QList>
#include "Descriptor.h"

namespace dai {

template <class T, int N>
class JointHistograms : public Descriptor
{
    QList<Histogram<T,N>> m_histograms;

public:
    JointHistograms(){}

    JointHistograms(const InstanceInfo &label, int frameId)
        : Descriptor(label, frameId)
    {
    }

    void addHistogram(const Histogram<T, N> &hist)
    {
         m_histograms.append(hist); // copy
    }

    float distance(const Descriptor& other_desc) const override
    {
        const JointHistograms& other = static_cast<const JointHistograms&>(other_desc);

        if (m_histograms.size() != other.m_histograms.size())
            return 1.0f;

        auto it1 = m_histograms.constBegin();
        auto it2 = other.m_histograms.constBegin();

        double distance = 0;

        while (it1 != m_histograms.constEnd() && it2 != other.m_histograms.constEnd())
        {
            distance += (*it1).distance(*it2);
            ++it1;
            ++it2;
        }

        return distance / m_histograms.size();
    }

    bool operator==(const Descriptor& other_desc) const override
    {
        const JointHistograms& other = static_cast<const JointHistograms&>(other_desc);

        if (m_histograms.size() == other.m_histograms.size())
            return false;

        bool equal = true;
        auto it1 = m_histograms.constBegin();
        auto it2 = other.m_histograms.constBegin();

        while (equal && it1 != m_histograms.constEnd() && it2 != other.m_histograms.constEnd())
        {
            equal = *it1 == *it2;
            ++it1;
            ++it2;
        }

        return equal;
    }

    int sizeInBytes() const
    {
        int size = sizeof(JointHistograms);

        for (auto hist: m_histograms)
            size += hist.sizeInBytes();

        return size;
    }
};

using JointHistograms1c = JointHistograms<uchar, 1>;
using JointHistograms1s = JointHistograms<ushort, 1>;

} // End Namespace


#endif // JOINT_HISTOGRAMS_H
