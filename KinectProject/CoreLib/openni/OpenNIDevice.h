#ifndef OPENNIDEVICE_H
#define OPENNIDEVICE_H

#include <OpenNI.h>
#include <NiTE.h>
#include <memory>
#include "types/DepthFrame.h"
#include "types/ColorFrame.h"
#include "types/SkeletonFrame.h"
#include "types/MaskFrame.h"
#include <QMutex>
#include <QHash>

namespace dai {

class OpenNIDevice
{
public:
    static void copySkeleton(const nite::Skeleton& srcSkeleton, dai::Skeleton& dstSkeleton);
    static OpenNIDevice* create(const QString devicePath = "ANY_DEVICE");

    ~OpenNIDevice();
    void open();
    void close();
    bool is_open() const;
    shared_ptr<ColorFrame> readColorFrame();
    shared_ptr<DepthFrame> readDepthFrame();
    shared_ptr<DepthFrame> depthFrame() const;
    shared_ptr<MaskFrame>  maskFrame() const;
    nite::UserTrackerFrameRef readUserTrackerFrame();
    openni::PlaybackControl* playbackControl();
    nite::UserTracker& getUserTracker();
    void depth2color(shared_ptr<DepthFrame> depthFrame, shared_ptr<MaskFrame> mask = nullptr);
    void setRegistration(bool flag);

private:
    OpenNIDevice(const QString devicePath);
    void initOpenNI();
    void shutdownOpenNI();

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
    nite::UserTracker          m_oniUserTracker;
    openni::VideoFrameRef      m_oniDepthFrame;
    shared_ptr<DepthFrame>     m_depthFrame;
    openni::VideoFrameRef      m_oniColorFrame;
    shared_ptr<ColorFrame>     m_colorFrame;
    shared_ptr<MaskFrame>      m_maskFrame;
    QMutex                     m_mutex;
    bool                       m_opened;
    bool                       m_manual_registration;
};

} // End Namespace

#endif // OPENNIDEVICE_H
