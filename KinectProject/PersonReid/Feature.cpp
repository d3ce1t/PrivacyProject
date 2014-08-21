#include "Feature.h"
#include <climits>

namespace dai {

shared_ptr<Feature> Feature::minFeature(const QList<shared_ptr<Feature>>& features)
{
    float min_distance = std::numeric_limits<float>::max();
    shared_ptr<Feature> selectedFeature;

    foreach (shared_ptr<Feature> feature1, features)
    {
        float distance = 0;

        foreach (shared_ptr<Feature> feature2, features)
        {
            distance += feature1->distance(*feature2);
        }

        if (distance < min_distance) {
            min_distance = distance;
            selectedFeature = feature1;
        }
    }

    return selectedFeature;
}

Feature::Feature(const Histogram1s& upper, const Histogram1s& lower, InstanceInfo label)
{
    m_upper_hist = upper;
    m_lower_hist = lower;
    m_label = label;
}

bool Feature::operator==(const Feature& other) const
{
    return m_upper_hist == other.m_upper_hist && m_lower_hist == other.m_lower_hist;
}

float Feature::distance(const Feature& other) const
{
    double distance = m_upper_hist.distance(other.m_upper_hist);
    distance += m_lower_hist.distance(other.m_lower_hist);
    return distance / 2;
}

 const InstanceInfo& Feature::label() const
 {
     return m_label;
 }

} // End Namespace
