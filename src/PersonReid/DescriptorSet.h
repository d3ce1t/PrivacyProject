#ifndef DESCRIPTORSET_H
#define DESCRIPTORSET_H

#include "Descriptor.h"
#include <QList>

namespace dai {

class DescriptorSet : public Descriptor
{
    QList<DescriptorPtr> m_descriptors;

public:
    //DescriptorSet();
    DescriptorSet(const InstanceInfo &label, int frameId);
    float distance(const Descriptor& other_desc) const override;
    bool operator==(const Descriptor& other) const override;
    void addDescriptor(DescriptorPtr descriptor);
};

} // End namespace

#endif // DESCRIPTORSET_H
