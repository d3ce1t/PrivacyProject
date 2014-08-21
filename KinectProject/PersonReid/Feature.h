#ifndef FEATURE_H
#define FEATURE_H

#include "dataset/InstanceInfo.h"
#include "types/Histogram.h"

namespace dai {

class Feature
{
    Histogram1s m_upper_hist;
    Histogram1s m_lower_hist;
    InstanceInfo m_label;

public:

    static shared_ptr<Feature> minFeature(const QList<shared_ptr<Feature> > &features);

    Feature(const Histogram1s& upper, const Histogram1s& lower, InstanceInfo label);
    float distance(const Feature& other) const;
    bool operator==(const Feature& other) const;
    const InstanceInfo& label() const;
};

} // End Namespace

#endif // FEATURE_H
