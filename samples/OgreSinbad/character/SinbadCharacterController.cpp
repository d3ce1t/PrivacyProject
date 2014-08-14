#include "character/SinbadCharacterController.h"

// Note: wont work as expected for > 5 users in scene
static unsigned int g_UsersColors[] = {/*0x70707080*/0 ,0x80FF0000,0x80FF4500,0x80FF1493,0x8000ff00, 0x8000ced1,0x80ffd700};
#define GetColorForUser(i) g_UsersColors[(i)%(sizeof(g_UsersColors)/sizeof(unsigned int))]

#define VALIDATE_GENERATOR(type, desc, generator)				\
{																\
    rc = m_Context.EnumerateExistingNodes(nodes, type);			\
    if (nodes.IsEmpty())										\
{															\
    printf("No %s generator!\n", desc);						\
    return 1;												\
}															\
    (*(nodes.Begin())).GetInstance(generator);					\
}
#define CHECK_RC(rc, what)											\
    if (rc != XN_STATUS_OK)											\
{																\
    printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
    return rc;													\
}

SinbadCharacterController::SinbadCharacterController(Camera* cam)
{
    m_poseTime = 0;
    m_candidateID = 0;
    m_poseCandidateID = 0;
    m_openni = nullptr;

    setupBody(cam->getSceneManager());
    setupCamera(cam);
    setupAnimations();

    // Init depth cam related stuff
    try {
        initPrimeSensor();
    }
    catch (int) {
        ErrorDialog dlg;
        dlg.display("Error initing sensor");
        exit(0);
    }
}

SinbadCharacterController::~SinbadCharacterController()
{
    m_openni->releaseInstance();
}

void SinbadCharacterController::UpdateDepthTexture()
{
    if (!m_oniUserTrackerFrame.isValid())
        return;

    TexturePtr texture = TextureManager::getSingleton().getByName("MyDepthTexture");
    // Get the pixel buffer
    HardwarePixelBufferSharedPtr pixelBuffer = texture->getBuffer();

    // Lock the pixel buffer and get a pixel box
    pixelBuffer->lock(HardwareBuffer::HBL_DISCARD);
    const PixelBox& pixelBox = pixelBuffer->getCurrentLock();

    unsigned char* pDest = static_cast<unsigned char*>(pixelBox.data);

    // Get label map
    const nite::UserMap& userMap = m_oniUserTrackerFrame.getUserMap();
    const nite::UserId* pUsersLBLs = userMap.getPixels();

    for (size_t j = 0; j < m_Height; j++)
    {
        pDest = static_cast<unsigned char*>(pixelBox.data) + j*pixelBox.rowPitch*4;

        for (size_t i = 0; i < m_Width; i++)
        {
            // fix i if we are mirrored
            unsigned int fixed_i = i;

            if(!m_front) {
                fixed_i = m_Width - i;
            }

            // determine color
            unsigned int color = GetColorForUser(pUsersLBLs[j*m_Width + fixed_i]);

            // if we have a candidate, filter out the rest
            if (m_poseCandidateID != 0 && m_poseCandidateID == pUsersLBLs[j*m_Width + fixed_i])
            {
                color = GetColorForUser(1);

                if( j > m_Height*(1 - m_detectionPercent) ) {
                    color |= 0xFF070707; //highlight user
                }/* else {
                        color &= 0x20F0F0F0; //hide user
                    }*/
            }
            else if (m_candidateID != 0 && m_candidateID == pUsersLBLs[j*m_Width + fixed_i])
            {
                color = GetColorForUser(1);
            }

            // write to output buffer
            *((unsigned int*)pDest) = color;
            pDest+=4;
        }
    }

    // Unlock the pixel buffer
    pixelBuffer->unlock();
}

void SinbadCharacterController::initPrimeSensor()
{
    qDebug() << "SinbadCharacterController::initPrimeSensor()";

    m_openni = dai::OpenNIRuntime::getInstance();
    m_openni->getDepthStream().setMirroringEnabled(m_front);

    // Skeleton stuff
    m_SmoothingFactor = 0.6;
    m_SmoothingDelta = 0;
    m_openni->getUserTracker().setSkeletonSmoothingFactor(m_SmoothingFactor);

    m_candidateID = 0;
}

void SinbadCharacterController::addTime(Real deltaTime)
{
    openniReadFrame();
    UpdateDepthTexture();
    updateBody(deltaTime);
    updateAnimations(deltaTime);
    PSupdateBody(deltaTime);
    updateCamera(deltaTime);
}

void SinbadCharacterController::openniReadFrame()
{
    nite::UserTracker& oniUserTracker = m_openni->getUserTracker();

    if (oniUserTracker.readFrame(&m_oniUserTrackerFrame) != nite::STATUS_OK) {
        throw 1;
    }

    if (!m_oniUserTrackerFrame.isValid()) {
        throw 2;
    }

    const nite::Array<nite::UserData>& users = m_oniUserTrackerFrame.getUsers();

    for (int i=0; i<users.getSize(); ++i)
    {
        const nite::UserData& user = users[i];

        // start looking for new users
        if (user.isNew())
        {
            qDebug()<< "New user!" << user.getId();
            oniUserTracker.startSkeletonTracking(user.getId());
            oniUserTracker.startPoseDetection(user.getId(), nite::POSE_PSI);
        }
        else if (!user.isLost())
        {
            // If we dont have an active candidate
            if (m_candidateID == 0)
            {
                const nite::PoseData& pose = user.getPose(nite::POSE_PSI);

                if (m_poseCandidateID == 0 && pose.isEntered())
                {
                    m_poseCandidateID = user.getId();
                    m_poseTime = m_oniUserTrackerFrame.getTimestamp();
                    qDebug() << "Pose Entered";
                }
                // PoseLost in OpenNI 1.5
                else if (user.getId() == m_poseCandidateID && pose.isExited())
                {
                    m_poseTime = 0;
                    m_poseCandidateID = 0;
                    m_detectionPercent = 0;
                    qDebug() << "Pose Exited";
                }
                else if (user.getId() == m_poseCandidateID && pose.isHeld())
                {
                    uint64_t currTime = m_oniUserTrackerFrame.getTimestamp();
                    m_detectionPercent = (currTime - m_poseTime) / (m_poseDuration * 1000.0f);
                    qDebug() << "Pose Held" << m_detectionPercent;

                    if (currTime - m_poseTime > m_poseDuration * 1000)
                    {
                        qDebug() << "Candidate Selected!";
                        m_candidateID = m_poseCandidateID;

                        const nite::Skeleton& oniSkeleton = user.getSkeleton();
                        const nite::SkeletonJoint& torsoJoint = oniSkeleton.getJoint(nite::JOINT_TORSO);

                        if (oniSkeleton.getState() == nite::SKELETON_TRACKED && torsoJoint.getPositionConfidence() > 0.5) {
                            qDebug() << "Initialised!!!";
                            m_origTorsoPos.x = -torsoJoint.getPosition().x;
                            m_origTorsoPos.y = torsoJoint.getPosition().y;
                            m_origTorsoPos.z = -torsoJoint.getPosition().z;
                        }
                    }
                }
            }
        }
        // Lost user
        else if (user.isLost())
        {
            qDebug() << "user lost" << user.getId();

            if (m_candidateID == user.getId()) {
                m_candidateID = 0;
                resetBonesToInitialState();
                m_poseCandidateID = 0;
                m_poseTime = 0;
                m_detectionPercent = 0;
            }

            if (m_poseCandidateID == user.getId()) {
                m_poseCandidateID = 0;
                m_poseTime = 0;
                m_detectionPercent = 0;
            }
        }
    } // End for
}

void SinbadCharacterController::injectKeyDown(const OIS::KeyEvent& evt)
{
    if (evt.key == OIS::KC_Q && (mTopAnimID == ANIM_IDLE_TOP || mTopAnimID == ANIM_RUN_TOP))
    {
        // take swords out (or put them back, since it's the same animation but reversed)
        setTopAnimation(ANIM_DRAW_SWORDS, true);
        mTimer = 0;
    }
    else if (evt.key == OIS::KC_E && !mSwordsDrawn)
    {
        if (mTopAnimID == ANIM_IDLE_TOP || mTopAnimID == ANIM_RUN_TOP)
        {
            // start dancing
            setBaseAnimation(ANIM_DANCE, true);
            setTopAnimation(ANIM_NONE);
            // disable hand animation because the dance controls hands
            mAnims[ANIM_HANDS_RELAXED]->setEnabled(false);
        }
        else if (mBaseAnimID == ANIM_DANCE)
        {
            // stop dancing
            setBaseAnimation(ANIM_IDLE_BASE);
            setTopAnimation(ANIM_IDLE_TOP);
            // re-enable hand animation
            mAnims[ANIM_HANDS_RELAXED]->setEnabled(true);
        }
    }

    //Smoothing Factor.
    if(evt.key == OIS::KC_H)
    {
        m_SmoothingDelta = 1;
    }
    else if(evt.key == OIS::KC_N)
    {
        m_SmoothingDelta = -1;
    }

    else if (evt.key == OIS::KC_SPACE && (mTopAnimID == ANIM_IDLE_TOP || mTopAnimID == ANIM_RUN_TOP))
    {
        // jump if on ground
        setBaseAnimation(ANIM_JUMP_START, true);
        setTopAnimation(ANIM_NONE);
        mTimer = 0;
    }

    if (!mKeyDirection.isZeroLength() && mBaseAnimID == ANIM_IDLE_BASE)
    {
        // start running if not already moving and the player wants to move
        setBaseAnimation(ANIM_RUN_BASE, true);
        if (mTopAnimID == ANIM_IDLE_TOP) setTopAnimation(ANIM_RUN_TOP, true);
    }
}

void SinbadCharacterController::injectMouseMove(const OIS::MouseEvent& evt)
{
    // update camera goal based on mouse movement
    updateCameraGoal(-0.05f * evt.state.X.rel, -0.05f * evt.state.Y.rel, -0.0005f * evt.state.Z.rel);
}

void SinbadCharacterController::injectMouseDown(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
{
    Q_UNUSED(evt)

    if (mSwordsDrawn && (mTopAnimID == ANIM_IDLE_TOP || mTopAnimID == ANIM_RUN_TOP))
    {
        // if swords are out, and character's not doing something weird, then SLICE!
        if (id == OIS::MB_Left) setTopAnimation(ANIM_SLICE_VERTICAL, true);
        else if (id == OIS::MB_Right) setTopAnimation(ANIM_SLICE_HORIZONTAL, true);
        mTimer = 0;
    }
}


void SinbadCharacterController::setupBody(SceneManager* sceneMgr)
{
    // create main model
    mBodyNode = sceneMgr->getRootSceneNode()->createChildSceneNode(Vector3::UNIT_Y * CHAR_HEIGHT);
    mBodyEnt = sceneMgr->createEntity("SinbadBody", "Sinbad.mesh");
    mBodyNode->attachObject(mBodyEnt);

    // create swords and attach to sheath
    mSword1 = sceneMgr->createEntity("SinbadSword1", "Sword.mesh");
    mSword2 = sceneMgr->createEntity("SinbadSword2", "Sword.mesh");
    mBodyEnt->attachObjectToBone("Sheath.L", mSword1);
    mBodyEnt->attachObjectToBone("Sheath.R", mSword2);

    // create a couple of ribbon trails for the swords, just for fun
    NameValuePairList params;
    params["numberOfChains"] = "2";
    params["maxElements"] = "80";
    mSwordTrail = (RibbonTrail*)sceneMgr->createMovableObject("RibbonTrail", &params);
    mSwordTrail->setMaterialName("Examples/LightRibbonTrail");
    mSwordTrail->setTrailLength(20);
    mSwordTrail->setVisible(false);
    sceneMgr->getRootSceneNode()->attachObject(mSwordTrail);

    for (int i = 0; i < 2; i++)
    {
        mSwordTrail->setInitialColour(i, 1, 0.8, 0);
        mSwordTrail->setColourChange(i, 0.75, 1.25, 1.25, 1.25);
        mSwordTrail->setWidthChange(i, 1);
        mSwordTrail->setInitialWidth(i, 0.5);
    }

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

    mSwordsDrawn = false;
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

void SinbadCharacterController::transformBone(const Ogre::String& modelBoneName, nite::JointType jointType)
{
    // Get the model skeleton bone info
    Skeleton* skel = mBodyEnt->getSkeleton();
    Ogre::Bone* bone = skel->getBone(modelBoneName);
    Ogre::Quaternion qI = bone->getInitialOrientation();
    Ogre::Quaternion newQ = Quaternion::IDENTITY;

    // Get the openNI bone info
    const nite::Skeleton& skeleton = m_oniUserTrackerFrame.getUserById(m_candidateID)->getSkeleton();
    const nite::SkeletonJoint& joint = skeleton.getJoint(jointType);

    if (joint.getOrientationConfidence() > 0 )
    {
        const nite::Quaternion& q = joint.getOrientation();
        newQ = Quaternion(-q.w, q.x, -q.y, q.z);

        bone->resetOrientation(); //in order for the conversion from world to local to work.
        newQ = bone->convertWorldToLocalOrientation(newQ);
        bone->setOrientation(newQ*qI);
    }
}

void SinbadCharacterController::PSupdateBody(Real deltaTime)
{
    Q_UNUSED(deltaTime)

    static bool bRightAfterSwardsPositionChanged = false;

    mGoalDirection = Vector3::ZERO;   // we will calculate this

    //set smoothing according to the players request.
    if (m_SmoothingDelta!=0)
    {
        m_SmoothingFactor += 0.01 * m_SmoothingDelta;

        if(m_SmoothingFactor >= 1)
            m_SmoothingFactor = 0.99;
        else if(m_SmoothingFactor <= 0)
            m_SmoothingFactor = 0.00;

        m_openni->getUserTracker().setSkeletonSmoothingFactor(m_SmoothingFactor);

        Ogre::DisplayString blah = "H/N ";
        blah.append(Ogre::StringConverter::toString((Real)m_SmoothingFactor));
    }

    if (!m_oniUserTrackerFrame.isValid())
        return;

    Skeleton* skel = mBodyEnt->getSkeleton();
    Ogre::Bone* rootBone = skel->getBone("Root");

    const nite::UserData* user = m_oniUserTrackerFrame.getUserById(this->m_candidateID);

    if (user && user->isVisible()) {

        const nite::Skeleton& oniSkeleton = user->getSkeleton();
        const nite::SkeletonJoint& torsoJoint = oniSkeleton.getJoint(nite::JOINT_TORSO);

        if (oniSkeleton.getState() == nite::SKELETON_TRACKED && torsoJoint.getPositionConfidence() > 0.5)
        {
            transformBone("Stomach", nite::JOINT_TORSO);
            transformBone("Waist", nite::JOINT_TORSO);
            transformBone("Root", nite::JOINT_TORSO);
            transformBone("Chest",nite::JOINT_TORSO);
            transformBone("Humerus.L",nite::JOINT_LEFT_SHOULDER);
            transformBone("Humerus.R",nite::JOINT_RIGHT_SHOULDER);
            transformBone("Ulna.L",nite::JOINT_LEFT_ELBOW);
            transformBone("Ulna.R",nite::JOINT_RIGHT_ELBOW);
            transformBone("Thigh.L",nite::JOINT_LEFT_HIP);
            transformBone("Thigh.R",nite::JOINT_RIGHT_HIP);
            transformBone("Calf.L",nite::JOINT_LEFT_KNEE);
            transformBone("Calf.R",nite::JOINT_RIGHT_KNEE);

            Vector3 newPos;
            newPos.x = -torsoJoint.getPosition().x;
            newPos.y = torsoJoint.getPosition().y;
            newPos.z = -torsoJoint.getPosition().z;

            newPos = (newPos - m_origTorsoPos);
            qDebug() << newPos.x << newPos.y << newPos.z;



            /*newPos.y -= 0.3;

            if (newPos.y < 0) {
                newPos.y /= 2.5;
                if (newPos.y < -1.5) {
                    newPos.y = -1.5;
                }
            }*/

            rootBone->setPosition(newPos);
        }

        //do gestures for swards
        if ((mTopAnimID == ANIM_IDLE_TOP || mTopAnimID == ANIM_RUN_TOP))
        {
            const nite::SkeletonJoint& jointHead = oniSkeleton.getJoint(nite::JOINT_HEAD);
            const nite::SkeletonJoint& jointLeftHand = oniSkeleton.getJoint(nite::JOINT_LEFT_HAND);
            const nite::SkeletonJoint& jointRightHand = oniSkeleton.getJoint(nite::JOINT_RIGHT_HAND);

            if (jointLeftHand.getPositionConfidence() > 0 && jointHead.getPositionConfidence() > 0)
            {
                dai::Vector3D leftVec = dai::Vector3D(jointLeftHand.getPosition().x,
                                                      jointLeftHand.getPosition().y,
                                                      jointLeftHand.getPosition().z);
                dai::Vector3D rightVec = dai::Vector3D(jointRightHand.getPosition().x,
                                                       jointRightHand.getPosition().y,
                                                       jointRightHand.getPosition().z);
                dai::Vector3D headVec = dai::Vector3D(jointHead.getPosition().x,
                                                      jointHead.getPosition().y,
                                                      jointHead.getPosition().z);

                dai::Vector3D tempVec = leftVec - rightVec;

                if (tempVec.lengthSquared() < 50000) {
                    tempVec = leftVec - headVec;

                    if(!bRightAfterSwardsPositionChanged &&
                            tempVec.lengthSquared() < 100000)
                    {
                        if(leftVec.z()+150 > headVec.z()) {
                            // take swords out (or put them back, since it's the same animation but reversed)
                            setTopAnimation(ANIM_DRAW_SWORDS, true);
                            mTimer = 0;
                            bRightAfterSwardsPositionChanged = true;
                        }
                    }
                }
                else {
                    bRightAfterSwardsPositionChanged = false;
                }
            }
        }
    } // end user
    else {
        rootBone->resetToInitialState();
    }
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

    if (mTopAnimID == ANIM_DRAW_SWORDS)
    {
        // flip the draw swords animation if we need to put it back
        topAnimSpeed = mSwordsDrawn ? -1 : 1;

        // half-way through the animation is when the hand grasps the handles...
        if (mTimer >= mAnims[mTopAnimID]->getLength() / 2 &&
                mTimer - deltaTime < mAnims[mTopAnimID]->getLength() / 2)
        {
            // so transfer the swords from the sheaths to the hands
            mBodyEnt->detachAllObjectsFromBone();
            mBodyEnt->attachObjectToBone(mSwordsDrawn ? "Sheath.L" : "Handle.L", mSword1);
            mBodyEnt->attachObjectToBone(mSwordsDrawn ? "Sheath.R" : "Handle.R", mSword2);
            // change the hand state to grab or let go
            mAnims[ANIM_HANDS_CLOSED]->setEnabled(!mSwordsDrawn);
            mAnims[ANIM_HANDS_RELAXED]->setEnabled(mSwordsDrawn);

            // toggle sword trails
            if (mSwordsDrawn)
            {
                mSwordTrail->setVisible(false);
                mSwordTrail->removeNode(mSword1->getParentNode());
                mSwordTrail->removeNode(mSword2->getParentNode());
            }
            else
            {
                mSwordTrail->setVisible(true);
                mSwordTrail->addNode(mSword1->getParentNode());
                mSwordTrail->addNode(mSword2->getParentNode());
            }
        }

        if (mTimer >= mAnims[mTopAnimID]->getLength())
        {
            // animation is finished, so return to what we were doing before
            if (mBaseAnimID == ANIM_IDLE_BASE) setTopAnimation(ANIM_IDLE_TOP);
            else
            {
                setTopAnimation(ANIM_RUN_TOP);
                mAnims[ANIM_RUN_TOP]->setTimePosition(mAnims[ANIM_RUN_BASE]->getTimePosition());
            }
            mSwordsDrawn = !mSwordsDrawn;
        }
    }
    else if (mTopAnimID == ANIM_SLICE_VERTICAL || mTopAnimID == ANIM_SLICE_HORIZONTAL)
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
