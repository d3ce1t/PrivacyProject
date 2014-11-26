#include "DescriptorSet.h"

namespace dai {

DescriptorSet::DescriptorSet()
{
}

DescriptorSet::DescriptorSet(const InstanceInfo &label, int frameId)
    : Descriptor(label, frameId)
{
}

float DescriptorSet::distance(const Descriptor& other_desc) const
{
    const DescriptorSet& other = static_cast<const DescriptorSet&>(other_desc);

    Q_ASSERT(m_descriptors.size() == other.m_descriptors.size());

    auto it1 = m_descriptors.constBegin();
    auto it2 = other.m_descriptors.constBegin();
    float distance = 0;

    while (it1 != m_descriptors.constEnd() && it2 != other.m_descriptors.constEnd())
    {
        distance += (*it1)->distance(*(*it2));

        // Move iterators
        ++it1;
        ++it2;
    }

    return distance / m_descriptors.size();
}

bool DescriptorSet::operator==(const Descriptor& other_desc) const
{
    const DescriptorSet& other = static_cast<const DescriptorSet&>(other_desc);

    auto it1 = m_descriptors.constBegin();
    auto it2 = other.m_descriptors.constBegin();
    bool equal = true;

    while (it1 != m_descriptors.constEnd() && it2 != other.m_descriptors.constEnd() && equal)
    {
        equal = *it1 == *it2;

        // Move iterators
        ++it1;
        ++it2;
    }

    return equal;
}

void DescriptorSet::addDescriptor(DescriptorPtr descriptor)
{
   m_descriptors << descriptor;
}

} // End Namespace
