#include "IASLAB_RGBD_ID_Instance.h"
#include "dataset/DatasetMetadata.h"
#include "types/ColorFrame.h"
#include "types/DepthFrame.h"
#include "types/MaskFrame.h"
#include "types/SkeletonFrame.h"
#include "openni/OpenNIDevice.h"
#include <opencv2/opencv.hpp>
#include <QFile>
#include "Utils.h"
#include <QDebug>

namespace dai {

SkeletonJoint::JointType IASLAB_RGBD_ID_Instance::_staticMap[20] = {
    SkeletonJoint::JOINT_SPINE,            // 0
    SkeletonJoint::JOINT_SPINE,            // 1
    SkeletonJoint::JOINT_CENTER_SHOULDER,  // 2
    SkeletonJoint::JOINT_HEAD,             // 3
    SkeletonJoint::JOINT_LEFT_SHOULDER,    // 4
    SkeletonJoint::JOINT_LEFT_ELBOW,       // 5
    SkeletonJoint::JOINT_LEFT_HAND,        // 6
    SkeletonJoint::JOINT_LEFT_HAND,        // 7
    SkeletonJoint::JOINT_RIGHT_SHOULDER,   // 8
    SkeletonJoint::JOINT_RIGHT_ELBOW,      // 9
    SkeletonJoint::JOINT_RIGHT_HAND,       // 10
    SkeletonJoint::JOINT_RIGHT_HAND,       // 11
    SkeletonJoint::JOINT_LEFT_HIP,         // 12
    SkeletonJoint::JOINT_LEFT_KNEE,        // 13
    SkeletonJoint::JOINT_LEFT_FOOT,        // 14
    SkeletonJoint::JOINT_LEFT_FOOT,        // 15
    SkeletonJoint::JOINT_RIGHT_HIP,        // 16
    SkeletonJoint::JOINT_RIGHT_KNEE,       // 17
    SkeletonJoint::JOINT_RIGHT_FOOT,       // 18
    SkeletonJoint::JOINT_RIGHT_FOOT        // 19
};

IASLAB_RGBD_ID_Instance::IASLAB_RGBD_ID_Instance(const InstanceInfo &info)
    : DataInstance(info, DataFrame::Color, -1, -1)
{
    m_width = 640;
    m_height = 480;
    m_open = false;
}

IASLAB_RGBD_ID_Instance::~IASLAB_RGBD_ID_Instance()
{
    m_width = 0;
    m_height = 0;
    closeInstance();
}

bool IASLAB_RGBD_ID_Instance::is_open() const
{
    return m_open;
}

bool IASLAB_RGBD_ID_Instance::hasNext() const
{
    return true;
}

bool IASLAB_RGBD_ID_Instance::openInstance()
{
    bool result = false;
    QString instancePath = m_info.parent().getPath() + "/";

    if (QFile::exists(instancePath + m_info.getFileName(DataFrame::Color)) &&
            QFile::exists(instancePath + m_info.getFileName(DataFrame::Depth)) &&
                QFile::exists(instancePath + m_info.getFileName(DataFrame::Mask)) &&
                    QFile::exists(instancePath + m_info.getFileName(DataFrame::Skeleton))) {
        result = true;
        m_open = true;
    }

    return result;
}

void IASLAB_RGBD_ID_Instance::closeInstance()
{
     m_open = false;
}

void IASLAB_RGBD_ID_Instance::restartInstance()
{
}

void IASLAB_RGBD_ID_Instance::nextFrame(QHashDataFrames &output)
{
    // Read Color File
    QString instancePath = m_info.parent().getPath() + "/" + m_info.getFileName(DataFrame::Color);
    cv::Mat color_mat = cv::imread(instancePath.toStdString());
    cv::cvtColor(color_mat, color_mat, CV_BGR2RGB);
    ColorFrame srcColor(color_mat.cols, color_mat.rows, (RGBColor*) color_mat.data);
    shared_ptr<ColorFrame> dstColor = make_shared<ColorFrame>();
    *dstColor = srcColor; // Copy
    output.insert(DataFrame::Color, dstColor);

    // Read Depth File
    instancePath = m_info.parent().getPath() + "/" + m_info.getFileName(DataFrame::Depth);
    QFile depthFile(instancePath);
    depthFile.open(QIODevice::ReadOnly);
    QByteArray buffer = depthFile.readAll();
    depthFile.close();
    shared_ptr<DepthFrame> depthFrame_tmp = make_shared<DepthFrame>(640, 480, (uint16_t*) (buffer.data() + 16));
    shared_ptr<DepthFrame> depthFrame = shared_ptr<DepthFrame>(new DepthFrame(*depthFrame_tmp)); // Clone!
    depthFrame->setDistanceUnits(dai::DISTANCE_MILIMETERS);
    output.insert(DataFrame::Depth, depthFrame);

    // Read Mask File
    instancePath = m_info.parent().getPath() + "/" + m_info.getFileName(DataFrame::Mask);
    QFile maskFile(instancePath);
    maskFile.open(QIODevice::ReadOnly);
    buffer = maskFile.readAll();
    maskFile.close();
    shared_ptr<MaskFrame> maskFrame_tmp = make_shared<MaskFrame>(640, 480, (uint8_t*) (buffer.data() + 14));
    shared_ptr<MaskFrame> maskFrame = shared_ptr<MaskFrame>(new MaskFrame(*maskFrame_tmp)); // Clone!
    output.insert(DataFrame::Mask, maskFrame);

    // Read Skeleton txt file (line by line)
    instancePath = m_info.parent().getPath() + "/" + m_info.getFileName(DataFrame::Skeleton);
    QFile skeletonFile(instancePath);
    skeletonFile.open(QIODevice::ReadOnly);
    QTextStream in(&skeletonFile);
    shared_ptr<Skeleton> skeleton = make_shared<Skeleton>();

    /*For every frame, a skeleton file is available. For every joint, a row with the following information is written to the skeleton file:
    [id]: person ID,
    [x3D], [y3D], [z3D]: joint 3D position,
    [x2D], [y2D]: joint 2D position,
    [TrackingState]: 0 (not tracked), 1 (inferred) or 2 (tracked),
    [QualityFlag]: not implemented by NiTE, thus it is always 0,
    [OrientationStartJoint], [OrientationEndJoint]: indices of the extreme joints of a link,
    [Qx], [Qy], [Qz], [Qw]: quaternion expressing the orientation of the link identified by [OrientationStartJoint] and [OrientationEndJoint].
    NiTE provides 15 joints, but these are remapped to the 20 joints that Microsoft Kinect SDK estimates, in order to have the same format (the joint positions missing in NiTE (wrists, ankles) are copied from other joints (hands, feet)).
    For more information, please visit http://msdn.microsoft.com/en-us/library/hh973074.aspx.
    You can also visualize the data with the "visualize_IASLab_RGBDID_dataset.m" Matlab script we provided.*/

    int lineCount = 0;

    QList<int> ignore_lines = {1, 7, 11, 15, 19};

    while (!in.atEnd())
    {
        QStringList columns = in.readLine().split(",");

        if (ignore_lines.contains(lineCount)) {
            lineCount++;
            continue;
        }

        SkeletonJoint joint(Point3f(columns[1].toFloat()*1000.0f, columns[2].toFloat()*1000.0f*-1, columns[3].toFloat()*1000.0f), _staticMap[lineCount]);
        int trackingState = columns[6].toInt();

        if (trackingState == 0)
            joint.setPositionConfidence(0.0f);

        joint.setOrientation(Quaternion(columns[13].toFloat(),
                             columns[10].toFloat(), columns[11].toFloat(), columns[12].toFloat()));

        //joint.setOrientationConfidence(niteJoint.getOrientationConfiden
        skeleton->setJoint(_staticMap[lineCount], joint);
        lineCount++;
    }

    skeletonFile.close();
    shared_ptr<SkeletonFrame> skeletonFrame = make_shared<SkeletonFrame>();
    skeletonFrame->setSkeleton(1, skeleton);
    output.insert(DataFrame::Skeleton, skeletonFrame);
}

} // End namespace
