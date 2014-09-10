#include "DistancesFeature.h"
#include "Utils.h"

namespace dai {

DistancesFeature::DistancesFeature()
{
}

DistancesFeature::DistancesFeature(const InstanceInfo &label, int frameId)
    : Descriptor(label, frameId)
{
}

float DistancesFeature::distance(const Descriptor& other_desc) const
{
    const DistancesFeature& other = static_cast<const DistancesFeature&>(other_desc);

    auto it1 = m_distances.constBegin();
    auto it2 = other.m_distances.constBegin();
    float distance = 0;

    while (it1 != m_distances.constEnd() && it2 != other.m_distances.constEnd())
    {
        distance += std::abs(*it1 - *it2);
        ++it1;
        ++it2;
    }

    return distance / m_distances.size();
}

bool DistancesFeature::operator==(const Descriptor& other_desc) const
{
    const DistancesFeature& other = static_cast<const DistancesFeature&>(other_desc);
    bool equal = true;

    auto it1 = m_distances.constBegin();
    auto it2 = other.m_distances.constBegin();

    while (equal && it1 != m_distances.constEnd() && it2 != other.m_distances.constEnd())
    {
        equal = *it1 == *it2;
        ++it1;
        ++it2;
    }

    return equal;
}

void DistancesFeature::addDistance(float value)
{
    m_distances << value;
}

} // End Namespace
