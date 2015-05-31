#ifndef OPENNIDEVICE_H
#define OPENNIDEVICE_H

#include <OpenNI.h>

#ifndef __APPLE__
#include <NiTE.h>
#endif

#include <memory>
#include "types/DepthFrame.h"
#include "types/ColorFrame.h"
#include "types/SkeletonFrame.h"
#include "types/MaskFrame.h"
#include "types/MetadataFrame.h"
#include <QMutex>
#include <QHash>

namespace dai {

class OpenNIDevice
{
    static QHash<QString, OpenNIDevice*> _created_instances;
    static SkeletonJoint::JointType _staticMap[15];
    static QMutex                   _mutex_counter;
    static int                      _instance_counter;
    static bool                     _initialised;

    QString                    m_devicePath;
    openni::Device             m_device;
    openni::VideoStream        m_oniDepthStream;
    openni::VideoStream        m_oniColorStream;
    openni::VideoStream**      m_depthStreams;
    openni::VideoStream**	   m_colorStreams;

#ifndef __APPLE__
    nite::UserTracker          m_oniUserTracker;
#endif

    openni::VideoFrameRef      m_oniDepthFrame;
    openni::VideoFrameRef      m_oniColorFrame;
    QMutex                     m_mutex;
    bool                       m_opened;
    bool                       m_manual_registration;
    int                        m_lastFrame;

public:
    static const QString ANY_DEVICE;
    static OpenNIDevice* create(const QString devicePath = OpenNIDevice::ANY_DEVICE);

#ifndef __APPLE__
    static void copySkeleton(const nite::Skeleton& srcSkeleton, dai::Skeleton& dstSkeleton);
#endif

    ~OpenNIDevice();
    void open();
    void close();
    bool is_open() const;
    bool hasNext();
    void readColorFrame(shared_ptr<ColorFrame> colorFrame);
    void readDepthFrame(shared_ptr<DepthFrame> depthFrame);
#ifndef __APPLE__
    void readUserTrackerFrame(shared_ptr<DepthFrame> depthFrame, shared_ptr<MaskFrame> maskFrame, shared_ptr<SkeletonFrame> skeletonFrame, shared_ptr<MetadataFrame> metadataFrame);
#endif
    openni::PlaybackControl* playbackControl();
    bool isFile() const;
    int getTotalFrames();
    void setRegistration(bool flag);
#ifndef __APPLE__
    void convertJointCoordinatesToDepth(float x, float y, float z, float* pOutX, float* pOutY) const;
    void convertDepthCoordinatesToJoint(int x, int y, int z, float* pOutX, float* pOutY) const;
#endif

private:
    OpenNIDevice(const QString devicePath);
    void initOpenNI();
    void shutdownOpenNI();
    void depth2color(shared_ptr<DepthFrame> depthFrame, shared_ptr<MaskFrame> mask = nullptr) const;
};

} // End Namespace

#endif // OPENNIDEVICE_H
