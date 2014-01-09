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

void SinbadCharacterController::lostUser(int userId)
{
    Q_UNUSED(userId);
    resetBonesToInitialState();
    mBodyEnt->setVisible(false);
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

void SinbadCharacterController::setupBody(SceneManager* sceneMgr)
{
    // create main model
    mBodyNode = sceneMgr->getRootSceneNode()->createChildSceneNode(Vector3::UNIT_Y * CHAR_HEIGHT);
    mBodyNode->scale(10, 11, 10);
    mBodyNode->setPosition(0, 0, -90);
    mBodyEnt = sceneMgr->createEntity("SinbadBody", "Sinbad.mesh");
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
    Vector3 xAxis, yAxis, zAxis;

    q.FromAngleAxis(Ogre::Degree(90),Vector3(0,0,-1));
    q.ToAxes(xAxis,yAxis,zAxis);
    q2.FromAngleAxis(Ogre::Degree(90),xAxis);
    setupBone("Humerus.L",q*q2);

    q.FromAngleAxis(Ogre::Degree(90),Vector3(0,0,1));
    q.ToAxes(xAxis,yAxis,zAxis);
    q2.FromAngleAxis(Ogre::Degree(90),xAxis);
    setupBone("Humerus.R",q*q2);

    q.FromAngleAxis(Ogre::Degree(90),Vector3(0,0,-1));
    q2.FromAngleAxis(Ogre::Degree(45),Vector3(0,-1,0));
    setupBone("Ulna.L",q*q2);

    q.FromAngleAxis(Ogre::Degree(90),Vector3(0,0,1));
    setupBone("Ulna.R",q*q2.Inverse());

    q.FromAngleAxis(Ogre::Degree(180),Vector3(0,1,0));
    setupBone("Chest",q);
    setupBone("Stomach",q);

    q.FromAngleAxis(Ogre::Degree(180),Vector3(1,0,0));
    q2.FromAngleAxis(Ogre::Degree(180),Vector3(0,1,0));
    setupBone("Thigh.L",q*q2);
    setupBone("Thigh.R",q*q2);
    setupBone("Calf.L",q*q2);
    setupBone("Calf.R",q*q2);
    setupBone("Root",Degree(0),Degree(0),Degree(0));

    // Setup Idle Base Animation
    mAnimIdle = mBodyEnt->getAnimationState("IdleBase");
    mAnimIdle->setLoop(true);

    // disable animation updates
    Animation* anim = mBodyEnt->getSkeleton()->getAnimation("IdleBase");
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Ulna.L")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Ulna.R")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Humerus.L")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Humerus.R")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Stomach")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Chest")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Thigh.L")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Thigh.R")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Calf.L")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Calf.R")->getHandle());
    anim->destroyNodeTrack(mBodyEnt->getSkeleton()->getBone("Root")->getHandle());
}

void SinbadCharacterController::resetBonesToInitialState()
{
    qDebug() << "resetBonesToInitialState";

    Skeleton* skel = mBodyEnt->getSkeleton();
    skel->getBone("Ulna.L")->resetToInitialState();
    skel->getBone("Ulna.R")->resetToInitialState();
    skel->getBone("Humerus.L")->resetToInitialState();
    skel->getBone("Humerus.R")->resetToInitialState();
    skel->getBone("Stomach")->resetToInitialState();
    skel->getBone("Chest")->resetToInitialState();
    skel->getBone("Thigh.L")->resetToInitialState();
    skel->getBone("Thigh.R")->resetToInitialState();
    skel->getBone("Calf.L")->resetToInitialState();
    skel->getBone("Calf.R")->resetToInitialState();
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
    transformBone("Stomach", dai::SkeletonJoint::JOINT_SPINE);
    transformBone("Waist", dai::SkeletonJoint::JOINT_SPINE);
    transformBone("Root", dai::SkeletonJoint::JOINT_SPINE);
    transformBone("Chest",dai::SkeletonJoint::JOINT_SPINE);
    transformBone("Humerus.L",dai::SkeletonJoint::JOINT_LEFT_SHOULDER);
    transformBone("Humerus.R",dai::SkeletonJoint::JOINT_RIGHT_SHOULDER);
    transformBone("Ulna.L",dai::SkeletonJoint::JOINT_LEFT_ELBOW);
    transformBone("Ulna.R",dai::SkeletonJoint::JOINT_RIGHT_ELBOW);
    transformBone("Thigh.L",dai::SkeletonJoint::JOINT_LEFT_HIP);
    transformBone("Thigh.R",dai::SkeletonJoint::JOINT_RIGHT_HIP);
    transformBone("Calf.L",dai::SkeletonJoint::JOINT_LEFT_KNEE);
    transformBone("Calf.R",dai::SkeletonJoint::JOINT_RIGHT_KNEE);

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
