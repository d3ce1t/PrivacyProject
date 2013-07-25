#include "MSRActionSkeletonInstance.h"
#include <QDebug>
#include <iostream>

using namespace std;

namespace dai {


MSRActionSkeletonInstance::MSRActionSkeletonInstance(const InstanceInfo& info)
    : DataInstance(info)
{
    m_frameBuffer[0].reset(new dai::Skeleton);
    m_frameBuffer[1].reset(new dai::Skeleton);
    DataInstance::initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
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

void MSRActionSkeletonInstance::openInstance()
{
    QString instancePath = m_info.getDatasetPath() + "/" + m_info.getFileName();

    if (!m_file.is_open())
    {
        m_file.open(instancePath.toStdString().c_str(), ios::in);

        if (!m_file.is_open()) {
            cerr << "Error opening file" << endl;
            return;
        }

        m_file.seekg(0, ios_base::beg);

        // Read Number of Frames from Depth File
        QString fileName = m_info.getFileName().replace("skeleton3D.txt", "sdepth.bin");
        QString depthPath = m_info.getDatasetPath() + "/" + fileName;
        ifstream depthFile;
        depthFile.open(depthPath.toStdString().c_str(), ios::in|ios::binary);

        if (!depthFile.is_open()) {
            cerr << "Error opening file" << endl;
            return;
        }

        depthFile.read((char *) &m_nFrames, 4);
        depthFile.close();

        m_nJoints = 20;
    }
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

void MSRActionSkeletonInstance::nextFrame(DataFrame &frame)
{
    dai::Skeleton& skeleton = (dai::Skeleton&) frame;

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

        SkeletonJoint joint(Point3f(w_x, w_y, w_z));
        SkeletonJoint::JointType type = convertIntToType(i);
        skeleton.setJoint(type, joint);
    }

    skeleton.computeQuaternions();
}

SkeletonJoint::JointType MSRActionSkeletonInstance::convertIntToType(int value)
{
    SkeletonJoint::JointType result = SkeletonJoint::JOINT_HEAD;

    switch (value) {
    case 0:
        result = SkeletonJoint::JOINT_RIGHT_SHOULDER; // Checked
        break;
    case 1:
        result = SkeletonJoint::JOINT_LEFT_SHOULDER; // Checked
        break;
    case 2:
        result = SkeletonJoint::JOINT_CENTER_SHOULDER; // Checked
        break;
    case 3:
        result = SkeletonJoint::JOINT_SPINE; // Checked
        break;
    case 4:
        result = SkeletonJoint::JOINT_RIGHT_HIP; // Checked
        break;
    case 5:
        result = SkeletonJoint::JOINT_LEFT_HIP; // Checked
        break;
    case 6:
        result = SkeletonJoint::JOINT_CENTER_HIP; // Checked
        break;
    case 7:
        result = SkeletonJoint::JOINT_RIGHT_ELBOW; // Checked
        break;
    case 8:
        result = SkeletonJoint::JOINT_LEFT_ELBOW; // Checked
        break;
    case 9:
        result = SkeletonJoint::JOINT_RIGHT_WRIST; // Checked
        break;
    case 10:
        result = SkeletonJoint::JOINT_LEFT_WRIST; // Checked
        break;
    case 11:
        result = SkeletonJoint::JOINT_RIGHT_HAND; // Checked
        break;
    case 12:
        result = SkeletonJoint::JOINT_LEFT_HAND; // Checked
        break;
    case 13:
        result = SkeletonJoint::JOINT_RIGHT_KNEE; // Checked
        break;
    case 14:
        result = SkeletonJoint::JOINT_LEFT_KNEE; // Checked
        break;
    case 15:
        result = SkeletonJoint::JOINT_RIGHT_ANKLE; // Checked
        break;
    case 16:
        result = SkeletonJoint::JOINT_LEFT_ANKLE; // Checked
        break;
    case 17:
        result = SkeletonJoint::JOINT_RIGHT_FOOT; // Checked
        break;
    case 18:
        result = SkeletonJoint::JOINT_LEFT_FOOT; // Checked
        break;
    case 19:
        result = SkeletonJoint::JOINT_HEAD; // Checked
        break;
    default:
        qDebug() << "No se deberia entrar aqui";
    }

    return result;
}

} // End Namespace
