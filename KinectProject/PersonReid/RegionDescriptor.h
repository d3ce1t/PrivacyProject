#ifndef REGION_DESCRIPTOR_H
#define REGION_DESCRIPTOR_H

#include <opencv2/features2d/features2d.hpp>
#include "Descriptor.h"

namespace dai {

class RegionDescriptor : public Descriptor
{
    QList<cv::Mat> m_descriptors;

public:
    RegionDescriptor();
    RegionDescriptor(const InstanceInfo &label, int frameId);
    float distance(const Descriptor& other) const override;
    bool operator==(const Descriptor& other) const override;
    void addDescriptor(const cv::Mat& descriptor);
};

} // End Namespace

#endif // REGION_DESCRIPTOR_H
