#ifndef DISTANCES_FEATURE_H
#define DISTANCES_FEATURE_H

#include "Descriptor.h"

namespace dai {

class DistancesFeature : public Descriptor
{
    QList<float> m_distances;

public:
    //DistancesFeature();
    DistancesFeature(const InstanceInfo &label, int frameId);
    float distance(const Descriptor& other) const override;
    bool operator==(const Descriptor& other) const override;
    void addDistance(float value);
    const QList<float>& distances() const {return m_distances;}
};

} // End Namespace

#endif // DISTANCES_FEATURE_H
