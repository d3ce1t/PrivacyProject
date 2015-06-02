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
#include <glm/glm.hpp>
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
    // Set Depth intrinsics of the camera that generated this frame
    depthFrame->setCameraIntrinsics(fx_d, cx_d, fy_d, cy_d);
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
    instancePath = m_info.parent().getPath() + "/" + m_info.getFileName(DataFrame::Skeleton);
    QFile skeletonFile(instancePath);
    skeletonFile.open(QIODevice::ReadOnly);
    QTextStream in(&skeletonFile);
    shared_ptr<Skeleton> skeleton = make_shared<Skeleton>();

    QList<int> ignore_lines = {1, 7, 11, 15, 19};
    int lineCount = 0;

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

    // Set intrinsics of the camera that generated this frame (depth camera in this case)
    skeleton->setCameraIntrinsics(fx_d, cx_d, fy_d, cy_d);
    shared_ptr<SkeletonFrame> skeletonFrame = make_shared<SkeletonFrame>();
    skeletonFrame->setSkeleton(1, skeleton);
    output.insert(DataFrame::Skeleton, skeletonFrame);

    // Register depth to color
    depth2color(depthFrame, maskFrame, skeleton);
}

void IASLAB_RGBD_ID_Instance::depth2color(shared_ptr<DepthFrame> depthFrame, shared_ptr<MaskFrame> mask, shared_ptr<Skeleton> skeleton) const
{
    /*const glm::mat3 r_matrix = {
        999.979, 6.497, -0.801,
        -6.498, 999.978, -1.054,
        0.794, 1.059, 999.999
    };*/

    // Translation Transform (millimeters)
    const glm::vec3 t_vector = {
        25, 0.0, 0.0
    };

    // Do Registration
    shared_ptr<DepthFrame> outputDepth = make_shared<DepthFrame>(640, 480);
    shared_ptr<MaskFrame> outputMask = make_shared<MaskFrame>(640, 480);

    // Because it will be alined to color image, it camera intrinsics are now those
    // of the colour camera
    outputDepth->setCameraIntrinsics(fx_rgb, cx_rgb, fy_rgb, cy_rgb);

    // Transform depth and mask
    for (int i=0; i<depthFrame->height(); ++i)
    {
        uint16_t* pDepth = depthFrame->getRowPtr(i);
        uint8_t* pMask = mask->getRowPtr(i);

        for (int j=0; j<depthFrame->width(); ++j)
        {
            // Convert each point of the depth frame into a real world coordinate in millimeters
            float out_z = pDepth[j];
            glm::vec3 p3d;
            p3d.x = (j - cx_d) * out_z / fx_d;
            p3d.y = (i - cy_d) * out_z / fy_d;
            p3d.z = out_z;

            // Rotate and Translate 3D point to change origin to color sensor
            //p3d = r_matrix * p3d;
            p3d = p3d + t_vector;

            // Reproject to 2D
            glm::vec2 p2d;
            p2d.x = (p3d.x * fx_rgb / p3d.z) + cx_rgb;
            p2d.y = (p3d.y * fy_rgb / p3d.z) + cy_rgb;

            if (p2d.x >= 0 && p2d.y >= 0 && p2d.x < 640 && p2d.y < 480) {
                outputDepth->setItem(p2d.y, p2d.x, pDepth[j]);
                outputMask->setItem(p2d.y, p2d.x, pMask[j]);
            }
        }
    }

    // Transform skeleton
    skeleton->setCameraIntrinsics(fx_rgb, cx_rgb, fy_rgb, cy_rgb);

    for (SkeletonJoint& joint : skeleton->joints())
    {
        glm::vec3 p3d_skel;
        p3d_skel.x = joint.getPosition()[0];
        p3d_skel.y = joint.getPosition()[1];
        p3d_skel.z = joint.getPosition()[2];
        // Translate (there is no rotation in this dataset)
        p3d_skel = p3d_skel + t_vector;
        joint.setPosition(Point3f(p3d_skel.x, p3d_skel.y, p3d_skel.z));
        skeleton->setJoint(joint.getType(), joint);
    }

    *depthFrame = *outputDepth; // Copy
    *mask = *outputMask; // Copy
}

} // End namespace
