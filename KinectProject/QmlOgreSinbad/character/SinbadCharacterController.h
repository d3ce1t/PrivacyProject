/*****************************************************************************
*                                                                            *
*  Sinbad Sample Application                                                 *
*  Copyright (C) 2010 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  OpenNI is free software: you can redistribute it and/or modify            *
*  it under the terms of the GNU Lesser General Public License as published  *
*  by the Free Software Foundation, either version 3 of the License, or      *
*  (at your option) any later version.                                       *
*                                                                            *
*  OpenNI is distributed in the hope that it will be useful,                 *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
*  GNU Lesser General Public License for more details.                       *
*                                                                            *
*  You should have received a copy of the GNU Lesser General Public License  *
*  along with OpenNI. If not, see <http://www.gnu.org/licenses/>.            *
*                                                                            *
*****************************************************************************/

// This sample is based on the Character sample from the OgreSDK.

#ifndef SINBAD_H
#define SINBAD_H

#include <QDebug>
#include "Ogre.h"
#include "openni/OpenNIRuntime.h"
#include <OgreStringConverter.h>
#include <OgreErrorDialog.h>

#define NUM_ANIMS 13           // number of animations the character has
#define CHAR_HEIGHT 5         // height of character's center of mass above ground
#define CAM_HEIGHT 1           // height of camera above character's center of mass
#define RUN_SPEED 17           // character running speed in units per second
#define TURN_SPEED 500.0f      // character turning in degrees per second
#define ANIM_FADE_SPEED 7.5f   // animation crossfade speed in % of full weight per second
#define JUMP_ACCEL 30.0f       // character jump acceleration in upward units per squared second
#define GRAVITY 90.0f          // gravity in downward units per squared second

using namespace Ogre;

class SinbadCharacterController
{
public:
    const unsigned int  m_Width = 640;
    const unsigned int m_Height = 480;

	double m_SmoothingFactor;
	int m_SmoothingDelta;
	bool m_front;	

    // time to hold in pose to exit program. In milliseconds.
    float m_detectionPercent = 0;
    const uint m_poseDuration = 3000;
    uint64_t m_poseTime;
    nite::UserId m_poseCandidateID;
    nite::UserId m_candidateID;
    nite::UserTrackerFrameRef m_oniUserTrackerFrame;
    Vector3 m_origTorsoPos;

    SinbadCharacterController(Camera* cam);
    ~SinbadCharacterController();
    void UpdateDepthTexture();
    void initPrimeSensor();
    void addTime(Real deltaTime);
    void openniReadFrame();
    /*void injectKeyDown(const OIS::KeyEvent& evt);
    void injectMouseMove(const OIS::MouseEvent& evt);
    void injectMouseDown(const OIS::MouseEvent& evt, OIS::MouseButtonID id);*/

private:
    // all the animations our character has, and a null ID
    // some of these affect separate body parts and will be blended together
    enum AnimID
    {
        ANIM_IDLE_BASE,
        ANIM_IDLE_TOP,
        ANIM_RUN_BASE,
        ANIM_RUN_TOP,
        ANIM_HANDS_CLOSED,
        ANIM_HANDS_RELAXED,
        ANIM_DRAW_SWORDS,
        ANIM_SLICE_VERTICAL,
        ANIM_SLICE_HORIZONTAL,
        ANIM_DANCE,
        ANIM_JUMP_START,
        ANIM_JUMP_LOOP,
        ANIM_JUMP_END,
        ANIM_NONE
    };

    void setupBody(SceneManager* sceneMgr);
    void setupBone(const String& name,const Ogre::Radian& angle, const Vector3 axis);
    void setupBone(const String& name,const Degree& yaw,const Degree& pitch,const Degree& roll);
    void setupBone(const String& name,const Ogre::Quaternion& q);
    void setupAnimations();
    void resetBonesToInitialState();
    void setupCamera(Camera* cam);
    void transformBone(const Ogre::String& modelBoneName, nite::JointType jointType);
    void PSupdateBody(Real deltaTime);
    void updateBody(Real deltaTime);
    void updateAnimations(Real deltaTime);
    void fadeAnimations(Real deltaTime);
    void updateCamera(Real deltaTime);
    void updateCameraGoal(Real deltaYaw, Real deltaPitch, Real deltaZoom);
    void setBaseAnimation(AnimID id, bool reset = false);
    void setTopAnimation(AnimID id, bool reset = false);

    dai::OpenNIRuntime* m_openni;
    Camera* mCamera;
    SceneNode* mBodyNode;
    SceneNode* mCameraPivot;
    SceneNode* mCameraGoal;
    SceneNode* mCameraNode;
    Real mPivotPitch;
    Entity* mBodyEnt;
    Entity* mSword1;
    Entity* mSword2;
    RibbonTrail* mSwordTrail;
    AnimationState* mAnims[NUM_ANIMS];    // master animation list
    AnimID mBaseAnimID;                   // current base (full- or lower-body) animation
    AnimID mTopAnimID;                    // current top (upper-body) animation
    bool mFadingIn[NUM_ANIMS];            // which animations are fading in
    bool mFadingOut[NUM_ANIMS];           // which animations are fading out
    bool mSwordsDrawn;
    Vector3 mKeyDirection;      // player's local intended direction based on WASD keys
    Vector3 mGoalDirection;     // actual intended direction in world-space
    Real mVerticalVelocity;     // for jumping
    Real mTimer;                // general timer to see how long animations have been playing
};

#endif
