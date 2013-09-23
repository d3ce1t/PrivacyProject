#include "MSRDailySkeletonInstance.h"
#include <QDebug>
#include <iostream>
#include "dataset/DatasetMetadata.h"

using namespace std;

namespace dai {

SkeletonJoint::JointType MSRDailySkeletonInstance::staticMap[20] = {
    SkeletonJoint::JOINT_CENTER_HIP,      // 0
    SkeletonJoint::JOINT_SPINE,           // 1
    SkeletonJoint::JOINT_CENTER_SHOULDER, // 2
    SkeletonJoint::JOINT_HEAD,            // 3
    SkeletonJoint::JOINT_RIGHT_SHOULDER,  // 4
    SkeletonJoint::JOINT_RIGHT_ELBOW,     // 5
    SkeletonJoint::JOINT_RIGHT_WRIST,     // 6
    SkeletonJoint::JOINT_RIGHT_HAND,      // 7
    SkeletonJoint::JOINT_LEFT_SHOULDER,   // 8
    SkeletonJoint::JOINT_LEFT_ELBOW,      // 9
    SkeletonJoint::JOINT_LEFT_WRIST,      // 10
    SkeletonJoint::JOINT_LEFT_HAND,       // 11
    SkeletonJoint::JOINT_RIGHT_HIP,       // 12
    SkeletonJoint::JOINT_RIGHT_KNEE,      // 13
    SkeletonJoint::JOINT_RIGHT_ANKLE,     // 14
    SkeletonJoint::JOINT_RIGHT_FOOT,      // 15
    SkeletonJoint::JOINT_LEFT_HIP,        // 16
    SkeletonJoint::JOINT_LEFT_KNEE,       // 17
    SkeletonJoint::JOINT_LEFT_ANKLE,      // 18
    SkeletonJoint::JOINT_LEFT_FOOT        // 19
};

MSRDailySkeletonInstance::MSRDailySkeletonInstance(const InstanceInfo& info)
    : DataInstance(info)
{
    m_frameBuffer[0].reset(new SkeletonFrame);
    m_frameBuffer[1].reset(new SkeletonFrame);
    DataInstance::initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
    m_nJoints = 0;
}

MSRDailySkeletonInstance::~MSRDailySkeletonInstance()
{
    closeInstance();
}

bool MSRDailySkeletonInstance::is_open() const
{
    return m_file.is_open();
}

bool MSRDailySkeletonInstance::openInstance()
{
    bool result = false;
    QString instancePath = m_info.parent().getPath() + "/" + m_info.getFileName();

    if (!m_file.is_open())
    {
        m_file.open(instancePath.toStdString().c_str(), ios::in);

        if (m_file.is_open())
        {
            m_file.seekg(0, ios_base::beg);
            m_file >> m_nFrames;
            m_file >> m_nJoints;
            result = true;
        }
    }

    return result;
}

void MSRDailySkeletonInstance::closeInstance()
{
    if (m_file.is_open()) {
        m_file.close();
    }
}

void MSRDailySkeletonInstance::restartInstance()
{
    if (m_file.is_open()) {
        m_file.seekg(0, ios_base::beg);
        m_file >> m_nFrames;
        m_file >> m_nJoints;
    }
}

void MSRDailySkeletonInstance::nextFrame(SkeletonFrame &frame)
{
    auto skeleton = frame.getSkeleton(1);

    if (skeleton == nullptr) {
        skeleton.reset(new Skeleton(Skeleton::SKELETON_KINECT));
        frame.setSkeleton(1, skeleton);
    }

    // Read Data from File
    int nRows = 0;
    m_file >> nRows;

    if (nRows != 40) {
        qDebug() << "Caso sin tratar " << nRows;
    }

    for (int i=0; i<nRows / 2; ++i)
    {
        float w_x, w_y, w_z; // World Coordinates
        float w_confidence;
        float s_x, s_y, s_z; // Screen Coordinates
        float s_confidence;

        // First Row
        m_file >> w_x;
        m_file >> w_y;
        m_file >> w_z;
        m_file >> w_confidence;

        // Second row
        m_file >> s_x;
        m_file >> s_y;
        m_file >> s_z;
        m_file >> s_confidence;

        SkeletonJoint joint(Point3f(w_x, w_y, w_z), staticMap[i]);
        skeleton->setJoint(staticMap[i], joint);
    }

    skeleton->computeQuaternions();
}

} // End Namespace
