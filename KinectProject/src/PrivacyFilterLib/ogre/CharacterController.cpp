#include "ogre/CharacterController.h"
#include <QDebug>

CharacterController::CharacterController(Ogre::Camera* cam)
    : m_enabled(false)
    , mBodyNode(nullptr)
    , mBodyEnt(nullptr)
    , m_userVisible(false)
{
    setupBody(cam->getSceneManager());
    setupAnimations();
}

void CharacterController::setSkeleton(shared_ptr<dai::Skeleton> skeleton)
{
    m_skeleton = skeleton;
}

void CharacterController::setVisible(bool visible)
{
    m_enabled = visible;

    // Hide char if it's visible and the filter is now disabled
    if (mBodyEnt) {
        mBodyEnt->setVisible(m_userVisible && m_enabled);
    }
}

void CharacterController::addTime(Ogre::Real deltaTime)
{
    updateBody(deltaTime);
}

void CharacterController::newUser(int userId)
{
    Q_UNUSED(userId);
    mBodyEnt->setVisible(m_enabled);
    m_userVisible = true;
}

void CharacterController::lostUser(int userId)
{
    Q_UNUSED(userId);
    resetBonesToInitialState();
    mBodyEnt->setVisible(false);
    m_userVisible = false;
}

void CharacterController::setupBody(Ogre::SceneManager* sceneMgr)
{
    // create main model
    mBodyNode = sceneMgr->getRootSceneNode()->createChildSceneNode("Human", Ogre::Vector3::UNIT_Y);
    //mBodyNode->scale(10.5, 10.5, 10.5);
    //mBodyNode->setPosition(0, 0, 0);
    mBodyNode->scale(1.0f/10, 1.0f/10.0f, 1.0f/10.0f);
    mBodyNode->setPosition(0, 0, -3.0f);
    mBodyEnt = sceneMgr->createEntity("HumanBody", "generic_male_01.mesh");
    mBodyEnt->setVisible(false);
    mBodyNode->attachObject(mBodyEnt);
    //Ogre::Vector3 size = mBodyEnt->getBoundingBox().getSize();
    //Ogre::Vector3 factor = mBodyNode->getScale();
    //qDebug() << factor.x*size.x << factor.y*size.y << factor.z*size.z;
}

void CharacterController::setupAnimations()
{
    // this is very important due to the nature of the exported animations
    mBodyEnt->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);

    Ogre::Quaternion q, q2;

    q.FromAngleAxis(Ogre::Degree(90), Ogre::Vector3(0, 0, -1));
    q2.FromAngleAxis(Ogre::Degree(180), Ogre::Vector3(0, -1, 0));
    setupBone("Upperarm.L", q*q2);

    q.FromAngleAxis(Ogre::Degree(90),Ogre::Vector3(0, 0, -1));
    q2.FromAngleAxis(Ogre::Degree(225),Ogre::Vector3(0, 1, 0));
    setupBone("Forearm.L", q*q2);

    q.FromAngleAxis(Ogre::Degree(90),Ogre::Vector3(0, 0, 1));
    q2.FromAngleAxis(Ogre::Degree(180),Ogre::Vector3(0, 1, 0));
    setupBone("Upperarm.R", q*q2);

    q.FromAngleAxis(Ogre::Degree(90),Ogre::Vector3(0, 0, 1));
    q2.FromAngleAxis(Ogre::Degree(225),Ogre::Vector3(0, -1, 0));
    setupBone("Forearm.R", q*q2);

    q.FromAngleAxis(Ogre::Degree(0),Ogre::Vector3(1, 0, 0));
    setupBone("Back",q);
    setupBone("Head", q);

    q.FromAngleAxis(Ogre::Degree(180),Ogre::Vector3(1, 0, 0));
    q2.FromAngleAxis(Ogre::Degree(0), Ogre::Vector3(0, 1, 0));
    setupBone("Hip", q*q2);
    setupBone("Thigh.L",q*q2);
    setupBone("Thigh.R",q*q2);
    setupBone("Shin.L",q*q2);
    setupBone("Shin.R",q*q2);

    q.FromAngleAxis(Ogre::Degree(90), Ogre::Vector3(-1, 0, 0));
    q2.FromAngleAxis(Ogre::Degree(180), Ogre::Vector3(0, 0, 1));
    setupBone("Root", q*q2);

    // Setup Idle Base Animation
    mAnimIdle = mBodyEnt->getAnimationState("Idle");
    mAnimIdle->setLoop(true);

    // disable animation updates
    Ogre::Animation* anim = mBodyEnt->getSkeleton()->getAnimation("Idle");
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Forearm.L")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Forearm.R")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Upperarm.L")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Upperarm.R")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Back")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Head")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Hip")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Thigh.L")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Thigh.R")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Shin.L")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Shin.R")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Root")->getHandle());
}

void CharacterController::resetBonesToInitialState()
{
    qDebug() << "resetBonesToInitialState";
    Ogre::Skeleton* skel = mBodyEnt->getSkeleton();
    skel->getBone("Forearm.L")->resetToInitialState();
    skel->getBone("Forearm.R")->resetToInitialState();
    skel->getBone("Upperarm.L")->resetToInitialState();
    skel->getBone("Upperarm.R")->resetToInitialState();
    skel->getBone("Back")->resetToInitialState();
    skel->getBone("Head")->resetToInitialState();
    skel->getBone("Hip")->resetToInitialState();
    skel->getBone("Thigh.L")->resetToInitialState();
    skel->getBone("Thigh.R")->resetToInitialState();
    skel->getBone("Shin.L")->resetToInitialState();
    skel->getBone("Shin.R")->resetToInitialState();
    skel->getBone("Root")->resetToInitialState();
}

void CharacterController::updateBody(Ogre::Real deltaTime)
{
    Q_UNUSED(deltaTime)

    // Update pose
    // Ogre Skeleton left-right is from camera perspective. DAI Skeleton left-right is from own skeleton
    transformBone("Head", dai::SkeletonJoint::JOINT_HEAD);
    transformBone("Back", dai::SkeletonJoint::JOINT_SPINE);
    transformBone("Hip", dai::SkeletonJoint::JOINT_SPINE);
    transformBone("Root", dai::SkeletonJoint::JOINT_SPINE);
    transformBone("Upperarm.L",dai::SkeletonJoint::JOINT_LEFT_SHOULDER);
    transformBone("Upperarm.R",dai::SkeletonJoint::JOINT_RIGHT_SHOULDER);
    transformBone("Forearm.L",dai::SkeletonJoint::JOINT_LEFT_ELBOW);
    transformBone("Forearm.R",dai::SkeletonJoint::JOINT_RIGHT_ELBOW);
    transformBone("Thigh.L",dai::SkeletonJoint::JOINT_LEFT_HIP);
    transformBone("Thigh.R",dai::SkeletonJoint::JOINT_RIGHT_HIP);
    transformBone("Shin.L",dai::SkeletonJoint::JOINT_LEFT_KNEE);
    transformBone("Shin.R",dai::SkeletonJoint::JOINT_RIGHT_KNEE);

    // Move character
    const dai::SkeletonJoint& torsoJoint = m_skeleton->getJoint(dai::SkeletonJoint::JOINT_SPINE);
    Ogre::Skeleton* skel = mBodyEnt->getSkeleton();
    Ogre::Vector3 newPos;

    if (m_skeleton->distanceUnits() == dai::DISTANCE_MILIMETERS) {
        newPos.x = torsoJoint.getPosition().val(0) / 1000.0f;
        newPos.y = torsoJoint.getPosition().val(1) / 1000.0f - 0.25;
        newPos.z = -(torsoJoint.getPosition().val(2) / 1000.0f + 3.2f);
    } else {
        newPos.x = torsoJoint.getPosition().val(0);
        newPos.y = torsoJoint.getPosition().val(1) - 0.25;
        newPos.z = -(torsoJoint.getPosition().val(2) + 3.2f);
    }

    skel->getBone("Root")->setPosition(newPos);
}

void CharacterController::setupBone(const Ogre::String &name, const Ogre::Radian &angle, const Ogre::Vector3 axis)
{
    Ogre::Quaternion q;
    q.FromAngleAxis(angle,axis);
    setupBone(name, q);
}

void CharacterController::setupBone(const Ogre::String &name, const Ogre::Degree &yaw,const Ogre::Degree &pitch,const Ogre::Degree &roll)
{
    Ogre::Bone* bone = mBodyEnt->getSkeleton()->getBone(name);
    bone->setManuallyControlled(true);
    bone->setInheritOrientation(false);
    bone->resetOrientation();
    bone->yaw(yaw);
    bone->pitch(pitch);
    bone->roll(roll);
    bone->setInitialState();
}

void CharacterController::setupBone(const Ogre::String &name, const Ogre::Quaternion &q)
{
    Ogre::Bone* bone = mBodyEnt->getSkeleton()->getBone(name);
    bone->setManuallyControlled(true);
    bone->setInheritOrientation(false);
    bone->resetOrientation();
    bone->setOrientation(q);
    bone->setInitialState();
}

void CharacterController::transformBone(const Ogre::String& modelBoneName, dai::SkeletonJoint::JointType jointType)
{
    // Get the model skeleton bone info
    Ogre::Skeleton* skel = mBodyEnt->getSkeleton();
    Ogre::Bone* bone = skel->getBone(modelBoneName);
    Ogre::Quaternion qI = bone->getInitialOrientation();
    Ogre::Quaternion newQ = Ogre::Quaternion::IDENTITY;

    // Get the OpenNI bone info
    const dai::SkeletonJoint& joint = m_skeleton->getJoint(jointType);

    if (joint.getOrientationConfidence() > 0 )
    {
        const dai::Quaternion& q = joint.getOrientation();
        newQ = Ogre::Quaternion(-q.w(), q.x(), q.y(), -q.z());
        bone->resetOrientation(); //in order for the conversion from world to local to work.
        newQ = bone->convertWorldToLocalOrientation(newQ);
        bone->setOrientation(newQ*qI);
    }
}
