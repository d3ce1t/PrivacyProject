#ifndef FEATURE_H
#define FEATURE_H

#include <memory>
#include "dataset/InstanceInfo.h"
#include "types/Histogram.h"
#include <QList>
#include <QImage>

namespace dai {

class Feature
{
    QList<Histogram2c> m_histograms;
    InstanceInfo m_label;
    int m_frameId;

public:

    static shared_ptr<Feature> minFeature(const QList<shared_ptr<Feature> > &features);
    static shared_ptr<Feature> minFeatureParallel(const QList<shared_ptr<Feature> > &features);

    Feature() {}
    Feature(InstanceInfo label, int frameId);
    void addHistogram(const Histogram2c &hist);
    float distance(const Feature& other) const;
    bool operator==(const Feature& other) const;
    const InstanceInfo& label() const;
    int frameId() const {return m_frameId;}
};


} // End Namespace

struct FeatureWrapper
{
    float distance;
    shared_ptr<dai::Feature> feature;
};



#endif // FEATURE_H
