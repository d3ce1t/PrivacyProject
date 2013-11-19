#include "ogre/SinbadCharacterController.h"

SinbadCharacterController::SinbadCharacterController(Camera* cam)
{
    setupBody(cam->getSceneManager());
    //setupCamera(cam);
    setupAnimations();
}

void SinbadCharacterController::setSkeleton(shared_ptr<dai::Skeleton> skeleton)
{
    m_skeleton = skeleton;
}

void SinbadCharacterController::addTime(Real deltaTime)
{
    updateBody(deltaTime);
    updateAnimations(deltaTime);
    PSupdateBody(deltaTime);
    //updateCamera(deltaTime);
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
    mVerticalVelocity = 0;
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

    //Matrix3 mat = bone->getLocalAxes();
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

    String animNames[] =
    {"IdleBase", "IdleTop", "RunBase", "RunTop", "HandsClosed", "HandsRelaxed", "DrawSwords",
     "SliceVertical", "SliceHorizontal", "Dance", "JumpStart", "JumpLoop", "JumpEnd"};

    //set all to manualy controlled
    //Ogre::Bone* handleLeft = mBodyEnt->getSkeleton()->getBone("Hand.L");
    //handleLeft->setManuallyControlled(true);

    //Ogre::Matrix3 mat;
    Ogre::Quaternion q = Quaternion::IDENTITY;
    Quaternion q2,q3;
    Vector3 xAxis,yAxis,zAxis;
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

    //rotate the body
    //mBodyNode->yaw(Degree(0),Node::TransformSpace::TS_WORLD);

    // populate our animation list
    for (int i = 0; i < NUM_ANIMS; i++)
    {
        mAnims[i] = mBodyEnt->getAnimationState(animNames[i]);
        mAnims[i]->setLoop(true);
        mFadingIn[i] = false;
        mFadingOut[i] = false;

        // disable animation updates
        Animation* anim = mBodyEnt->getSkeleton()->getAnimation(animNames[i]);

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

    // start off in the idle state (top and bottom together)
    setBaseAnimation(ANIM_IDLE_BASE);
    setTopAnimation(ANIM_IDLE_TOP);

    // relax the hands since we're not holding anything
    mAnims[ANIM_HANDS_RELAXED]->setEnabled(true);
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

void SinbadCharacterController::setupCamera(Camera* cam)
{
    // create a pivot at roughly the character's shoulder
    mCameraPivot = cam->getSceneManager()->getRootSceneNode()->createChildSceneNode();
    // this is where the camera should be soon, and it spins around the pivot
    mCameraGoal = mCameraPivot->createChildSceneNode(Vector3(0, 0, 15));
    // this is where the camera actually is
    mCameraNode = cam->getSceneManager()->getRootSceneNode()->createChildSceneNode();
    mCameraNode->setPosition(mCameraPivot->getPosition() + mCameraGoal->getPosition());

    mCameraPivot->setFixedYawAxis(true);
    mCameraGoal->setFixedYawAxis(true);
    mCameraNode->setFixedYawAxis(true);

    // our model is quite small, so reduce the clipping planes
    cam->setNearClipDistance(0.1);
    cam->setFarClipDistance(100);
    mCameraNode->attachObject(cam);

    m_front=false;

    mPivotPitch = 0;
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

    if (mKeyDirection != Vector3::ZERO && mBaseAnimID != ANIM_DANCE)
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
        if (mBaseAnimID == ANIM_JUMP_LOOP) yawAtSpeed *= 0.2f;

        // turn as much as we can, but not more than we need to
        if (yawToGoal < 0) yawToGoal = std::min<Real>(0, std::max<Real>(yawToGoal, yawAtSpeed)); //yawToGoal = Math::Clamp<Real>(yawToGoal, yawAtSpeed, 0);
        else if (yawToGoal > 0) yawToGoal = std::max<Real>(0, std::min<Real>(yawToGoal, yawAtSpeed)); //yawToGoal = Math::Clamp<Real>(yawToGoal, 0, yawAtSpeed);


        mBodyNode->yaw(Degree(yawToGoal));

        // move in current body direction (not the goal direction)
        mBodyNode->translate(0, 0, deltaTime * RUN_SPEED * mAnims[mBaseAnimID]->getWeight(),
                             Node::TS_LOCAL);
    }

    if (mBaseAnimID == ANIM_JUMP_LOOP)
    {
        // if we're jumping, add a vertical offset too, and apply gravity
        mBodyNode->translate(0, mVerticalVelocity * deltaTime, 0, Node::TS_LOCAL);
        mVerticalVelocity -= GRAVITY * deltaTime;

        Vector3 pos = mBodyNode->getPosition();
        if (pos.y <= CHAR_HEIGHT)
        {
            // if we've hit the ground, change to landing state
            pos.y = CHAR_HEIGHT;
            mBodyNode->setPosition(pos);
            setBaseAnimation(ANIM_JUMP_END, true);
            mTimer = 0;
        }
    }
}

void SinbadCharacterController::updateAnimations(Real deltaTime)
{
    Real baseAnimSpeed = 1;
    Real topAnimSpeed = 1;

    mTimer += deltaTime;

    if (mTopAnimID == ANIM_SLICE_VERTICAL || mTopAnimID == ANIM_SLICE_HORIZONTAL)
    {
        if (mTimer >= mAnims[mTopAnimID]->getLength())
        {
            // animation is finished, so return to what we were doing before
            if (mBaseAnimID == ANIM_IDLE_BASE) setTopAnimation(ANIM_IDLE_TOP);
            else
            {
                setTopAnimation(ANIM_RUN_TOP);
                mAnims[ANIM_RUN_TOP]->setTimePosition(mAnims[ANIM_RUN_BASE]->getTimePosition());
            }
        }

        // don't sway hips from side to side when slicing. that's just embarrasing.
        if (mBaseAnimID == ANIM_IDLE_BASE) baseAnimSpeed = 0;
    }
    else if (mBaseAnimID == ANIM_JUMP_START)
    {
        if (mTimer >= mAnims[mBaseAnimID]->getLength())
        {
            // takeoff animation finished, so time to leave the ground!
            setBaseAnimation(ANIM_JUMP_LOOP, true);
            // apply a jump acceleration to the character
            mVerticalVelocity = JUMP_ACCEL;
        }
    }
    else if (mBaseAnimID == ANIM_JUMP_END)
    {
        if (mTimer >= mAnims[mBaseAnimID]->getLength())
        {
            // safely landed, so go back to running or idling
            if (mKeyDirection == Vector3::ZERO)
            {
                setBaseAnimation(ANIM_IDLE_BASE);
                setTopAnimation(ANIM_IDLE_TOP);
            }
            else
            {
                setBaseAnimation(ANIM_RUN_BASE, true);
                setTopAnimation(ANIM_RUN_TOP, true);
            }
        }
    }

    // increment the current base and top animation times
    if (mBaseAnimID != ANIM_NONE) mAnims[mBaseAnimID]->addTime(deltaTime * baseAnimSpeed);
    if (mTopAnimID != ANIM_NONE) mAnims[mTopAnimID]->addTime(deltaTime * topAnimSpeed);

    // apply smooth transitioning between our animations
    fadeAnimations(deltaTime);
}

void SinbadCharacterController::fadeAnimations(Real deltaTime)
{
    for (int i = 0; i < NUM_ANIMS; i++)
    {
        if (mFadingIn[i])
        {
            // slowly fade this animation in until it has full weight
            Real newWeight = mAnims[i]->getWeight() + deltaTime * ANIM_FADE_SPEED;
            mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
            if (newWeight >= 1) mFadingIn[i] = false;
        }
        else if (mFadingOut[i])
        {
            // slowly fade this animation out until it has no weight, and then disable it
            Real newWeight = mAnims[i]->getWeight() - deltaTime * ANIM_FADE_SPEED;
            mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
            if (newWeight <= 0)
            {
                mAnims[i]->setEnabled(false);
                mFadingOut[i] = false;
            }
        }
    }
}

void SinbadCharacterController::updateCamera(Real deltaTime)
{
    // place the camera pivot roughly at the character's shoulder
    mCameraPivot->setPosition(mBodyNode->getPosition() + Vector3::UNIT_Y * CAM_HEIGHT + Vector3::UNIT_Z*4);
    // move the camera smoothly to the goal
    Vector3 goalOffset = mCameraGoal->_getDerivedPosition() - mCameraNode->getPosition();
    mCameraNode->translate(goalOffset * deltaTime * 9.0f);
    // always look at the pivot
    mCameraNode->lookAt(mCameraPivot->_getDerivedPosition(), Node::TS_WORLD);
}

void SinbadCharacterController::updateCameraGoal(Real deltaYaw, Real deltaPitch, Real deltaZoom)
{
    mCameraPivot->yaw(Degree(deltaYaw), Node::TS_WORLD);

    // bound the pitch
    if (!(mPivotPitch + deltaPitch > 25 && deltaPitch > 0) &&
            !(mPivotPitch + deltaPitch < -60 && deltaPitch < 0))
    {
        mCameraPivot->pitch(Degree(deltaPitch), Node::TS_LOCAL);
        mPivotPitch += deltaPitch;
    }

    Real dist = mCameraGoal->_getDerivedPosition().distance(mCameraPivot->_getDerivedPosition());
    Real distChange = deltaZoom * dist;

    // bound the zoom
    if (!(dist + distChange < 8 && distChange < 0) &&
            !(dist + distChange > 25 && distChange > 0))
    {
        mCameraGoal->translate(0, 0, distChange, Node::TS_LOCAL);
    }
}

void SinbadCharacterController::setBaseAnimation(AnimID id, bool reset)
{
    if (mBaseAnimID >= 0 && mBaseAnimID < NUM_ANIMS)
    {
        // if we have an old animation, fade it out
        mFadingIn[mBaseAnimID] = false;
        mFadingOut[mBaseAnimID] = true;
    }

    mBaseAnimID = id;

    if (id != ANIM_NONE)
    {
        // if we have a new animation, enable it and fade it in
        mAnims[id]->setEnabled(true);
        mAnims[id]->setWeight(0);
        mFadingOut[id] = false;
        mFadingIn[id] = true;
        if (reset) mAnims[id]->setTimePosition(0);
    }
}

void SinbadCharacterController::setTopAnimation(AnimID id, bool reset)
{
    qDebug() << "setTopAnimation";

    if (mTopAnimID >= 0 && mTopAnimID < NUM_ANIMS)
    {
        // if we have an old animation, fade it out
        mFadingIn[mTopAnimID] = false;
        mFadingOut[mTopAnimID] = true;
    }

    mTopAnimID = id;

    if (id != ANIM_NONE)
    {
        // if we have a new animation, enable it and fade it in
        mAnims[id]->setEnabled(true);
        mAnims[id]->setWeight(0);
        mFadingOut[id] = false;
        mFadingIn[id] = true;
        if (reset) mAnims[id]->setTimePosition(0);
    }
}
