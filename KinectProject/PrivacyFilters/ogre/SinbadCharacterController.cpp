#include "ogre/SinbadCharacterController.h"
#include <QDebug>

SinbadCharacterController::SinbadCharacterController(Camera* cam)
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
    // Q_UNUSED(userId);
    qDebug() << "Sinbad: new user" << userId;
    const dai::SkeletonJoint& jointTorso = m_skeleton->getJoint(dai::SkeletonJoint::JOINT_SPINE);
    m_origTorsoPos.x = jointTorso.getPosition().x();
    m_origTorsoPos.y = jointTorso.getPosition().y();
    m_origTorsoPos.z = -jointTorso.getPosition().z();
    mBodyEnt->setVisible(true);
}

void SinbadCharacterController::lostUser(int userId)
{
    Q_UNUSED(userId);
    resetBonesToInitialState();
    mBodyEnt->setVisible(false);
}

void SinbadCharacterController::setupBody(SceneManager* sceneMgr)
{
    // create main model
    mBodyNode = sceneMgr->getRootSceneNode()->createChildSceneNode(Vector3::UNIT_Y);
    mBodyNode->scale(23, 23, 23);
    mBodyNode->setPosition(0, 0, 0);
    mBodyEnt = sceneMgr->createEntity("SinbadBody", "meHumanMale.mesh");
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

    Quaternion q = Quaternion::IDENTITY;
    Quaternion q2;

    q.FromAngleAxis(Ogre::Degree(90), Vector3(0, 0, -1));
    q2.FromAngleAxis(Ogre::Degree(90), Vector3(0, -1, 0));
    setupBone("Upperarm.Left", q*q2);

    q.FromAngleAxis(Ogre::Degree(90),Vector3(0, 0, 1));
    q2.FromAngleAxis(Ogre::Degree(90),Vector3(0, 1, 0));
    setupBone("Upperarm.Right", q*q2);

    q.FromAngleAxis(Ogre::Degree(90),Vector3(0, 0, -1));
    q2.FromAngleAxis(Ogre::Degree(90),Vector3(0, -1, 0));
    setupBone("Forearm.Left", q*q2);

    q.FromAngleAxis(Ogre::Degree(90),Vector3(0, 0, 1));
    q2.FromAngleAxis(Ogre::Degree(90),Vector3(0, 1, 0));
    setupBone("Forearm.Right", q*q2);

    q.FromAngleAxis(Ogre::Degree(0),Vector3(0, 0, 1));
    setupBone("Back",q);

    q.FromAngleAxis(Ogre::Degree(180),Vector3(1, 0, 0));
    q2.FromAngleAxis(Ogre::Degree(0), Vector3(0, 1, 0));
    setupBone("Hip", q*q2);
    setupBone("Thigh.Left",q*q2);
    setupBone("Thigh.Right",q*q2);
    setupBone("Shin.Left",q*q2);
    setupBone("Shin.Right",q*q2);

    q.FromAngleAxis(Ogre::Degree(-45), Vector3(1, 0, 0));
    setupBone("COG", q);

    // Setup Idle Base Animation
    mAnimIdle = mBodyEnt->getAnimationState("Idle-M");
    mAnimIdle->setLoop(true);

    // disable animation updates
    Animation* anim = mBodyEnt->getSkeleton()->getAnimation("Idle-M");
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Forearm.Left")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Forearm.Right")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Upperarm.Left")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Upperarm.Right")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Back")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Hip")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Thigh.Left")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Thigh.Right")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Shin.Left")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Shin.Right")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("COG")->getHandle());
}

void SinbadCharacterController::resetBonesToInitialState()
{
    qDebug() << "resetBonesToInitialState";
    Skeleton* skel = mBodyEnt->getSkeleton();
    skel->getBone("Forearm.Left")->resetToInitialState();
    skel->getBone("Forearm.Right")->resetToInitialState();
    skel->getBone("Upperarm.Left")->resetToInitialState();
    skel->getBone("Upperarm.Right")->resetToInitialState();
    skel->getBone("Back")->resetToInitialState();
    skel->getBone("Hip")->resetToInitialState();
    skel->getBone("Thigh.Left")->resetToInitialState();
    skel->getBone("Thigh.Right")->resetToInitialState();
    skel->getBone("Shin.Left")->resetToInitialState();
    skel->getBone("Shin.Right")->resetToInitialState();
    //skel->getBone("COG")->resetToInitialState();
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
    Ogre::Bone* rootBone = skel->getBone("COG");

    const dai::SkeletonJoint& torsoJoint = m_skeleton->getJoint(dai::SkeletonJoint::JOINT_SPINE);

    // Ogre Skeleton left-right is from camera perspective. DAI Skeleton left-right is from own skeleton
    transformBone("Back", dai::SkeletonJoint::JOINT_SPINE);
    transformBone("Hip", dai::SkeletonJoint::JOINT_SPINE);
    //transformBone("COG", dai::SkeletonJoint::JOINT_SPINE);
    transformBone("Upperarm.Left",dai::SkeletonJoint::JOINT_LEFT_SHOULDER);
    transformBone("Upperarm.Right",dai::SkeletonJoint::JOINT_RIGHT_SHOULDER);
    transformBone("Forearm.Left",dai::SkeletonJoint::JOINT_LEFT_ELBOW);
    transformBone("Forearm.Right",dai::SkeletonJoint::JOINT_RIGHT_ELBOW);
    transformBone("Thigh.Left",dai::SkeletonJoint::JOINT_LEFT_HIP);
    transformBone("Thigh.Right",dai::SkeletonJoint::JOINT_RIGHT_HIP);
    transformBone("Shin.Left",dai::SkeletonJoint::JOINT_LEFT_KNEE);
    transformBone("Shin.Right",dai::SkeletonJoint::JOINT_RIGHT_KNEE);

    Vector3 newPos;
    newPos.x = torsoJoint.getPosition().x();
    newPos.y = torsoJoint.getPosition().y();
    newPos.z = -torsoJoint.getPosition().z();
    newPos = (newPos - m_origTorsoPos) * 5;

    newPos.y -= 0.3;

    if (newPos.y < 0) {
        newPos.y /= 2.5;
        if (newPos.y < -1.5) {
            newPos.y = -1.5;
        }
    }

    qDebug() << "Confidence" << torsoJoint.getPositionConfidence() << torsoJoint.getPosition().x() << torsoJoint.getPosition().y() << torsoJoint.getPosition().z();
    qDebug() << "Bone Depth" << rootBone->getPosition().x << rootBone->getPosition().y << rootBone->getPosition().z;
    //rootBone->setPosition(newPos);
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
        //mBodyNode->translate(0, 0, deltaTime * RUN_SPEED * mAnimIdle->getWeight(), Node::TS_LOCAL);
    }
}
