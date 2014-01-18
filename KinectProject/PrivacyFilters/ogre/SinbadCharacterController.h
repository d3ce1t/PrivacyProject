#ifndef SINBAD_H
#define SINBAD_H

#include "Ogre.h"
#include "openni/OpenNIRuntime.h"
#include <OgreStringConverter.h>
#include <OgreErrorDialog.h>

#define RUN_SPEED   17         // character running speed in units per second
#define TURN_SPEED  500.0f     // character turning in degrees per second

using namespace Ogre;

class SinbadCharacterController
{
public:
    SinbadCharacterController(Camera* cam);
    void addTime(Real deltaTime);
    void setSkeleton(shared_ptr<dai::Skeleton> skeleton);
    void newUser(int userId);
    void lostUser(int userId);

private:
    void setupBody(SceneManager* sceneMgr);
    void setupBone(const String& name,const Ogre::Radian& angle, const Vector3 axis);
    void setupBone(const String& name,const Degree& yaw,const Degree& pitch,const Degree& roll);
    void setupBone(const String& name,const Ogre::Quaternion& q);
    void setupAnimations();
    void resetBonesToInitialState();
    void transformBone(const Ogre::String& modelBoneName, dai::SkeletonJoint::JointType jointType);
    void PSupdateBody(Real deltaTime);
    void updateBody(Real deltaTime);    

    Vector3 m_origTorsoPos;
    AnimationState* mAnimIdle;
    SceneNode* mBodyNode;
    SceneNode* mCameraNode;
    Entity* mBodyEnt;
    Vector3 mKeyDirection;      // player's local intended direction based on WASD keys
    Vector3 mGoalDirection;     // actual intended direction in world-space
    Real mTimer;                // general timer to see how long animations have been playing
    shared_ptr<dai::Skeleton> m_skeleton;
};

#endif
