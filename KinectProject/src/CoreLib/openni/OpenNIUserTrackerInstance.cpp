#include "OpenNIUserTrackerInstance.h"
#include <exception>
#include <iostream>
#include <QDebug>

using namespace std;

namespace dai {

OpenNIUserTrackerInstance::OpenNIUserTrackerInstance()
    : StreamInstance(DataFrame::Depth | DataFrame::Mask | DataFrame::Skeleton | DataFrame::Metadata, 640, 480)
{
    m_device = nullptr;
}

OpenNIUserTrackerInstance::OpenNIUserTrackerInstance(OpenNIDevice* device)
    : StreamInstance(DataFrame::Depth | DataFrame::Mask | DataFrame::Skeleton | DataFrame::Metadata, 640, 480)
{
    m_device = device;
}

OpenNIUserTrackerInstance::~OpenNIUserTrackerInstance()
{
    closeInstance();
}

bool OpenNIUserTrackerInstance::is_open() const
{
    return m_device->is_open();
}

bool OpenNIUserTrackerInstance::openInstance()
{
    bool result = false;

    if (!is_open())
    {
        if (m_device == nullptr)
            m_device = OpenNIDevice::create("ANY_DEVICE");
        m_device->open();
        result = true;
    }

    return result;
}

void OpenNIUserTrackerInstance::closeInstance()
{
    if (is_open()) {
        m_device->close();
    }
}

void OpenNIUserTrackerInstance::restartInstance()
{
}

void OpenNIUserTrackerInstance::nextFrame(QHashDataFrames& output)
{
    Q_ASSERT(output.size() > 0);
#ifndef __APPLE__
    shared_ptr<DepthFrame> depthFrame = static_pointer_cast<DepthFrame>(output.value(DataFrame::Depth));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(output.value(DataFrame::Mask));
    shared_ptr<SkeletonFrame> skeletonFrame = static_pointer_cast<SkeletonFrame>(output.value(DataFrame::Skeleton));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(output.value(DataFrame::Metadata));
    m_device->readUserTrackerFrame(depthFrame, maskFrame, skeletonFrame, metadataFrame);
#endif
}

} // End Namespace
