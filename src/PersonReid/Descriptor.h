#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include "dataset/InstanceInfo.h"
#include <memory>
#include <QList>

namespace dai {

class Descriptor;
using DescriptorPtr = shared_ptr<Descriptor>;

class Descriptor
{
protected:
    InstanceInfo m_label;
    int m_frameId;

public:
    static float minDistanceParallel(const DescriptorPtr feature, const QList<DescriptorPtr>& samples);
    static DescriptorPtr minFeature(const QList<DescriptorPtr> &features);
    static DescriptorPtr minFeatureParallel(const QList<DescriptorPtr> &features);

    //Descriptor();
    Descriptor(const InstanceInfo &label, int frameId);
    virtual float distance(const Descriptor& other) const = 0;
    virtual bool operator==(const Descriptor& other) const;
    const InstanceInfo& label() const {return m_label;}
    int frameId() const {return m_frameId;}
};

} // End Namespace

#endif // DESCRIPTOR_H
