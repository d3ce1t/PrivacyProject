#ifndef JOINTSURF_H
#define JOINTSURF_H

#include <opencv2/features2d/features2d.hpp>
#include "Descriptor.h"

namespace dai {

class JointSurf : public Descriptor
{
    QList<cv::Mat> m_descriptors;

public:
    JointSurf();
    JointSurf(const InstanceInfo &label, int frameId);
    float distance(const Descriptor& other) const override;
    bool operator==(const Descriptor& other) const override;
    void addSurfDescriptor(const cv::Mat& descriptor);
};

} // End Namespace

#endif // JOINTSURF_H
