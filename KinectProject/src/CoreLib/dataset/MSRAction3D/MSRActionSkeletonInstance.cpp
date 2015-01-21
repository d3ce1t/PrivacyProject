#include "MSRActionSkeletonInstance.h"
#include <QDebug>
#include <iostream>
#include "dataset/DatasetMetadata.h"

using namespace std;

namespace dai {

SkeletonJoint::JointType MSRActionSkeletonInstance::staticMap[20] = {
    SkeletonJoint::JOINT_RIGHT_SHOULDER,  // 0
    SkeletonJoint::JOINT_LEFT_SHOULDER,   // 1
    SkeletonJoint::JOINT_CENTER_SHOULDER, // 2
    SkeletonJoint::JOINT_SPINE,           // 3
    SkeletonJoint::JOINT_RIGHT_HIP,       // 4
    SkeletonJoint::JOINT_LEFT_HIP,        // 5
    SkeletonJoint::JOINT_CENTER_HIP,      // 6
    SkeletonJoint::JOINT_RIGHT_ELBOW,     // 7
    SkeletonJoint::JOINT_LEFT_ELBOW,      // 8
    SkeletonJoint::JOINT_RIGHT_WRIST,     // 9
    SkeletonJoint::JOINT_LEFT_WRIST,      // 10
    SkeletonJoint::JOINT_RIGHT_HAND,      // 11
    SkeletonJoint::JOINT_LEFT_HAND,       // 12
    SkeletonJoint::JOINT_RIGHT_KNEE,      // 13
    SkeletonJoint::JOINT_LEFT_KNEE,       // 14
    SkeletonJoint::JOINT_RIGHT_ANKLE,     // 15
    SkeletonJoint::JOINT_LEFT_ANKLE,      // 16
    SkeletonJoint::JOINT_RIGHT_FOOT,      // 17
    SkeletonJoint::JOINT_LEFT_FOOT,       // 18
    SkeletonJoint::JOINT_HEAD             // 19
};

MSRActionSkeletonInstance::MSRActionSkeletonInstance(const InstanceInfo& info)
    : DataInstance(info, DataFrame::Skeleton, 320, 240)
{
    m_nJoints = 0;
}

MSRActionSkeletonInstance::~MSRActionSkeletonInstance()
{
    closeInstance();
}

bool MSRActionSkeletonInstance::is_open() const
{
    return m_file.is_open();
}

bool MSRActionSkeletonInstance::openInstance()
{
    bool result = false;
    QString datasetPath = m_info.parent().getPath();
    QString instancePath = datasetPath + "/" + m_info.getFileName(DataFrame::Skeleton);

    if (!m_file.is_open())
    {
        m_file.open(instancePath.toStdString().c_str(), ios::in);

        if (m_file.is_open())
        {
            m_file.seekg(0, ios_base::beg);

            // Read Number of Frames from Depth File
            QString fileName = m_info.getFileName(DataFrame::Skeleton).replace("skeleton3D.txt", "sdepth.bin");
            QString depthPath = datasetPath + "/" + fileName;
            ifstream depthFile;
            depthFile.open(depthPath.toStdString().c_str(), ios::in|ios::binary);

            if (depthFile.is_open()) {
                depthFile.read((char *) &m_nFrames, 4);
                depthFile.close();
                result = true;
            }
        }

        m_nJoints = 20;
    }

    return result;
}

void MSRActionSkeletonInstance::closeInstance()
{
    if (m_file.is_open()) {
        m_file.close();
    }
}

void MSRActionSkeletonInstance::restartInstance()
{
    if (m_file.is_open()) {
        m_file.seekg(0, ios_base::beg);
    }
}

void MSRActionSkeletonInstance::nextFrame(QHashDataFrames &output)
{
    Q_ASSERT(output.size() > 0);
    shared_ptr<SkeletonFrame> skeletonFrame = static_pointer_cast<SkeletonFrame>(output.value(DataFrame::Skeleton));
    auto skeleton = skeletonFrame->getSkeleton(1);

    if (skeleton == nullptr) {
        skeleton = make_shared<Skeleton>(Skeleton::SKELETON_KINECT);
        skeleton->setDistanceUnits(dai::METERS);
        skeletonFrame->setSkeleton(1, skeleton);
    }

    // Read Data from File
    int nRows = m_nJoints;

    for (int i=0; i<nRows; ++i)
    {
        float w_x, w_y, w_z; // World Coordinates
        float w_confidence;

        // First Row
        m_file >> w_x;
        m_file >> w_y;
        m_file >> w_z;
        m_file >> w_confidence;

        SkeletonJoint joint(Point3f(w_x, w_y, w_z), staticMap[i]);
        joint.setPositionConfidence(w_confidence);
        skeleton->setJoint(staticMap[i], joint);
    }

    skeleton->computeQuaternions();
}

} // End Namespace
