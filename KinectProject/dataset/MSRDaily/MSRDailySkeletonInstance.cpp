#include "MSRDailySkeletonInstance.h"
#include <QDebug>
#include <iostream>

using namespace std;

namespace dai {

MSRDailySkeletonInstance::MSRDailySkeletonInstance(const InstanceInfo& info)
    : DataInstance(info)
{
    m_frameBuffer[0].reset(new dai::Skeleton);
    m_frameBuffer[1].reset(new dai::Skeleton);
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

void MSRDailySkeletonInstance::openInstance()
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

        m_file >> m_nFrames;
        m_file >> m_nJoints;
    }
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

void MSRDailySkeletonInstance::nextFrame(DataFrame &frame)
{
    dai::Skeleton& skeleton = (dai::Skeleton&) frame;

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

        SkeletonJoint joint(Point3f(w_x, w_y, w_z));
        SkeletonJoint::JointType type = convertIntToType(i);
        skeleton.setJoint(type, joint);
    }

    skeleton.computeQuaternions();
}

SkeletonJoint::JointType MSRDailySkeletonInstance::convertIntToType(int value)
{
    SkeletonJoint::JointType result = SkeletonJoint::JOINT_HEAD;

    switch (value) {
    case 0:
        result = SkeletonJoint::JOINT_CENTER_HIP;       // Checked
        break;
    case 1:
        result = SkeletonJoint::JOINT_SPINE;    // Checked
        break;
    case 2:
        result = SkeletonJoint::JOINT_CENTER_SHOULDER; // Checked
        break;
    case 3:
        result = SkeletonJoint::JOINT_HEAD; // Checked
        break;
    case 4:
        result = SkeletonJoint::JOINT_RIGHT_SHOULDER; // Checked
        break;
    case 5:
        result = SkeletonJoint::JOINT_RIGHT_ELBOW; // Checked
        break;
    case 6:
        result = SkeletonJoint::JOINT_RIGHT_WRIST; // Checked
        break;
    case 7:
        result = SkeletonJoint::JOINT_RIGHT_HAND; // Checked
        break;
    case 8:
        result = SkeletonJoint::JOINT_LEFT_SHOULDER; // Checked
        break;
    case 9:
        result = SkeletonJoint::JOINT_LEFT_ELBOW; // Checked
        break;
    case 10:
        result = SkeletonJoint::JOINT_LEFT_WRIST; // Checked
        break;
    case 11:
        result = SkeletonJoint::JOINT_LEFT_HAND; // Checked
        break;
    case 12:
        result = SkeletonJoint::JOINT_RIGHT_HIP;
        break;
    case 13:
        result = SkeletonJoint::JOINT_RIGHT_KNEE;
        break;
    case 14:
        result = SkeletonJoint::JOINT_RIGHT_ANKLE;
        break;
    case 15:
        result = SkeletonJoint::JOINT_RIGHT_FOOT;
        break;
    case 16:
        result = SkeletonJoint::JOINT_LEFT_HIP;
        break;
    case 17:
        result = SkeletonJoint::JOINT_LEFT_KNEE;
        break;
    case 18:
        result = SkeletonJoint::JOINT_LEFT_ANKLE;
        break;
    case 19:
        result = SkeletonJoint::JOINT_LEFT_FOOT;
        break;
    default:
        qDebug() << "No se deberia entrar aqui";
    }

    return result;
}

} // End Namespace
