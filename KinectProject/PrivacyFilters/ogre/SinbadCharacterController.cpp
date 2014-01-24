#include "ogre/SinbadCharacterController.h"
#include <QDebug>

SinbadCharacterController::SinbadCharacterController(Camera* cam)
    : m_enabled(false)
    , mBodyNode(nullptr)
    , mCameraNode(nullptr)
    , mBodyEnt(nullptr)
    , m_userVisible(false)
{
    setupBody(cam->getSceneManager());
    setupAnimations();
}

void SinbadCharacterController::addTime(Real deltaTime)
{
    updateBody(deltaTime);
    PSupdateBody(deltaTime);
}

void SinbadCharacterController::setSkeleton(shared_ptr<dai::Skeleton> skeleton)
{
    m_skeleton = skeleton;
}

void SinbadCharacterController::newUser(int userId)
{
    Q_UNUSED(userId);
    const dai::SkeletonJoint& jointTorso = m_skeleton->getJoint(dai::SkeletonJoint::JOINT_SPINE);
    m_origTorsoPos.x = jointTorso.getPosition().x();
    m_origTorsoPos.y = jointTorso.getPosition().y();
    m_origTorsoPos.z = -jointTorso.getPosition().z();
    mBodyEnt->setVisible(m_enabled);
    m_userVisible = true;
}

void SinbadCharacterController::lostUser(int userId)
{
    Q_UNUSED(userId);
    resetBonesToInitialState();
    mBodyEnt->setVisible(false);
    m_userVisible = false;
}

void SinbadCharacterController::setVisible(bool visible)
{
    m_enabled = visible;

    // Hide char if it's visible and the filter is now disabled
    if (mBodyEnt) {
        mBodyEnt->setVisible(m_userVisible && m_enabled);
    }
}

void SinbadCharacterController::setupBody(SceneManager* sceneMgr)
{
    // create main model
    mBodyNode = sceneMgr->getRootSceneNode()->createChildSceneNode("Human", Vector3::UNIT_Y);
    mBodyNode->scale(10.5, 10, 10);
    mBodyNode->setPosition(0, 0, 0);
    mBodyEnt = sceneMgr->createEntity("HumanBody", "generic_male_01.mesh");
    mBodyEnt->setVisible(false);
    mBodyNode->attachObject(mBodyEnt);
    mKeyDirection = Vector3::ZERO;
}

void SinbadCharacterController::setupBone(const String& name,const Ogre::Radian& angle, const Vector3 axis)
{
    Quaternion q;
    q.FromAngleAxis(angle,axis);
    setupBone(name, q);
}

void SinbadCharacterController::setupBone(const String& name,const Degree& yaw,const Degree& pitch,const Degree& roll)
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

void SinbadCharacterController::setupBone(const String& name,const Ogre::Quaternion& q)
{
    Ogre::Bone* bone = mBodyEnt->getSkeleton()->getBone(name);
    bone->setManuallyControlled(true);
    bone->setInheritOrientation(false);
    bone->resetOrientation();
    bone->setOrientation(q);
    bone->setInitialState();
}

void SinbadCharacterController::setupAnimations()
{
    // this is very important due to the nature of the exported animations
    mBodyEnt->getSkeleton()->setBlendMode(ANIMBLEND_CUMULATIVE);

    Quaternion q, q2;

    q.FromAngleAxis(Ogre::Degree(90), Vector3(0, 0, -1));
    q2.FromAngleAxis(Ogre::Degree(180), Vector3(0, -1, 0));
    setupBone("Upperarm.L", q*q2);

    q.FromAngleAxis(Ogre::Degree(90),Vector3(0, 0, -1));
    q2.FromAngleAxis(Ogre::Degree(225),Vector3(0, 1, 0));
    setupBone("Forearm.L", q*q2);

    q.FromAngleAxis(Ogre::Degree(90),Vector3(0, 0, 1));
    q2.FromAngleAxis(Ogre::Degree(180),Vector3(0, 1, 0));
    setupBone("Upperarm.R", q*q2);

    q.FromAngleAxis(Ogre::Degree(90),Vector3(0, 0, 1));
    q2.FromAngleAxis(Ogre::Degree(225),Vector3(0, -1, 0));
    setupBone("Forearm.R", q*q2);

    q.FromAngleAxis(Ogre::Degree(0),Vector3(1, 0, 0));
    setupBone("Back",q);
    setupBone("Head", q);

    q.FromAngleAxis(Ogre::Degree(180),Vector3(1, 0, 0));
    q2.FromAngleAxis(Ogre::Degree(0), Vector3(0, 1, 0));
    setupBone("Hip", q*q2);
    setupBone("Thigh.L",q*q2);
    setupBone("Thigh.R",q*q2);
    setupBone("Shin.L",q*q2);
    setupBone("Shin.R",q*q2);

    q.FromAngleAxis(Ogre::Degree(90), Vector3(-1, 0, 0));
    q2.FromAngleAxis(Ogre::Degree(180), Vector3(0, 0, 1));
    setupBone("Root", q*q2);

    // Setup Idle Base Animation
    mAnimIdle = mBodyEnt->getAnimationState("Idle");
    mAnimIdle->setLoop(true);

    // disable animation updates
    Animation* anim = mBodyEnt->getSkeleton()->getAnimation("Idle");
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

void SinbadCharacterController::resetBonesToInitialState()
{
    qDebug() << "resetBonesToInitialState";
    Skeleton* skel = mBodyEnt->getSkeleton();
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

void SinbadCharacterController::transformBone(const Ogre::String& modelBoneName, dai::SkeletonJoint::JointType jointType)
{
    // Get the model skeleton bone info
    Skeleton* skel = mBodyEnt->getSkeleton();
    Ogre::Bone* bone = skel->getBone(modelBoneName);
    Ogre::Quaternion qI = bone->getInitialOrientation();
    Ogre::Quaternion newQ = Quaternion::IDENTITY;

    // Get the openNI bone info
    const dai::SkeletonJoint& joint = m_skeleton->getJoint(jointType);

    if (joint.getOrientationConfidence() > 0 )
    {
        const dai::Quaternion& q = joint.getOrientation();
        newQ = Quaternion(-q.w(), q.x(), q.y(), -q.z());
        bone->resetOrientation(); //in order for the conversion from world to local to work.
        newQ = bone->convertWorldToLocalOrientation(newQ);
        bone->setOrientation(newQ*qI);
    }
}

void SinbadCharacterController::PSupdateBody(Real deltaTime)
{
    Q_UNUSED(deltaTime)

    mGoalDirection = Vector3::ZERO;   // we will calculate this
    Skeleton* skel = mBodyEnt->getSkeleton();
    Ogre::Bone* rootBone = skel->getBone("Root");

    const dai::SkeletonJoint& torsoJoint = m_skeleton->getJoint(dai::SkeletonJoint::JOINT_SPINE);

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

    Vector3 newPos;
    newPos.x = torsoJoint.getPosition().x();
    newPos.y = torsoJoint.getPosition().y() - 0.25;
    newPos.z = -(torsoJoint.getPosition().z() + 3.2f);
    rootBone->setPosition(newPos);
}

void SinbadCharacterController::updateBody(Real deltaTime)
{
    mGoalDirection = Vector3::ZERO;   // we will calculate this

    if (mKeyDirection != Vector3::ZERO)
    {
        // calculate actually goal direction in world based on player's key directions
        mGoalDirection += mKeyDirection.z * mCameraNode->getOrientation().zAxis();
        mGoalDirection += mKeyDirection.x * mCameraNode->getOrientation().xAxis();
        mGoalDirection.y = 0;
        mGoalDirection.normalise();

        Quaternion toGoal = mBodyNode->getOrientation().zAxis().getRotationTo(mGoalDirection);

        // calculate how much the character has to turn to face goal direction
        Real yawToGoal = toGoal.getYaw().valueDegrees();
        // this is how much the character CAN turn this frame
        Real yawAtSpeed = yawToGoal / Math::Abs(yawToGoal) * deltaTime * TURN_SPEED;
        // reduce "turnability" if we're in midair

        // turn as much as we can, but not more than we need to
        if (yawToGoal < 0) yawToGoal = std::min<Real>(0, std::max<Real>(yawToGoal, yawAtSpeed)); //yawToGoal = Math::Clamp<Real>(yawToGoal, yawAtSpeed, 0);
        else if (yawToGoal > 0) yawToGoal = std::max<Real>(0, std::min<Real>(yawToGoal, yawAtSpeed)); //yawToGoal = Math::Clamp<Real>(yawToGoal, 0, yawAtSpeed);

        mBodyNode->yaw(Degree(yawToGoal));

        // move in current body direction (not the goal direction)
        mBodyNode->translate(0, 0, deltaTime * RUN_SPEED * mAnimIdle->getWeight(), Node::TS_LOCAL);
    }
}
