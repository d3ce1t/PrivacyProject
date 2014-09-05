#ifndef FEATURE_H
#define FEATURE_H

#include "dataset/InstanceInfo.h"
#include "types/Histogram.h"
#include <QList>

namespace dai {

class Feature
{
    QList<Histogram1s> m_histograms;
    InstanceInfo m_label;
    int m_frameId;

public:

    static shared_ptr<Feature> minFeature(const QList<shared_ptr<Feature> > &features);

    Feature() {}
    Feature(InstanceInfo label, int frameId);
    void addHistogram(const Histogram1s& hist);
    float distance(const Feature& other) const;
    bool operator==(const Feature& other) const;
    const InstanceInfo& label() const;
    int frameId() const {return m_frameId;}
};

} // End Namespace

#endif // FEATURE_H
