#include "DAISkeletonInstance.h"

#include "DAIDepthInstance.h"
#include <iostream>

namespace dai {

DAISkeletonInstance::DAISkeletonInstance(const InstanceInfo& info)
    : DataInstance(info)
{
    m_frameBuffer[0].reset(new SkeletonFrame);
    m_frameBuffer[1].reset(new SkeletonFrame);
    DataInstance::initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
}

DAISkeletonInstance::~DAISkeletonInstance()
{
    closeInstance();
}

bool DAISkeletonInstance::is_open() const
{
    return m_file.is_open();
}

void DAISkeletonInstance::openInstance()
{
    QString instancePath = m_info.getDatasetPath() + "/" + m_info.getFileName();

    if (!m_file.is_open())
    {
        m_file.open(instancePath.toStdString().c_str(), ios::in|ios::binary);

        if (!m_file.is_open()) {
            cerr << "Error opening file" << endl;
            return;
        }

        m_file.seekg(0, ios_base::beg);
        m_file.read((char *) &m_nFrames, 4);
    }
}

void DAISkeletonInstance::closeInstance()
{
    if (m_file.is_open()) {
        m_file.close();
    }
}

void DAISkeletonInstance::restartInstance()
{
    if (m_file.is_open()) {
        m_file.seekg(4, ios_base::beg);
    }
}

void DAISkeletonInstance::nextFrame(SkeletonFrame &frame)
{
    // Read Data from File
    frame.clear();

    // Read frame index
    unsigned int frameId;
    m_file.read( (char*) &frameId, sizeof(frameId) );

    // Read Num Skeletons
    int numSkeletons;
    m_file.read( (char*) &numSkeletons, sizeof(numSkeletons) );

    // Read all skeletons
    for (int i=0; i<numSkeletons; ++i)
    {
        // Write user
        int userId;
        m_file.read( (char*) &userId, sizeof(userId) );

        // Write Skeleton Type (determine used joints)
        Skeleton::SkeletonType type;
        m_file.read( (char*) &type, sizeof(Skeleton::SkeletonType) );

        // Create skeleton for user with the read type
        shared_ptr<Skeleton> skeleton = frame.getSkeleton(userId);

        if (skeleton == nullptr) {
            skeleton.reset(new Skeleton(type));
            frame.setSkeleton(userId, skeleton);
        }

        // Read all joints (by now, used and unused)
        for (int i=0; i<MAX_JOINTS; ++i) {
            Point3f position;
            m_file.read( (char*) position.dataPtr(), 3 * sizeof(double) );
            SkeletonJoint joint(position, (SkeletonJoint::JointType) i);
            skeleton->setJoint( (SkeletonJoint::JointType) i, joint);
        }

        //skeleton->computeQuaternions();
    }
}

} // End namespace

