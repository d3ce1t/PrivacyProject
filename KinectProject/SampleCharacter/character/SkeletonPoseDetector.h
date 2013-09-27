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

#pragma once

#include <math.h>
#include <time.h>
#include <NiTE.h>

enum PoseDetectionResult
{
	NOT_IN_POSE,
	IN_POSE_FOR_LITTLE_TIME,
	IN_POSE_FOR_LONG_TIME,
    NOT_ENOUGH_CONFIDENCE
};

static double GetCurrentTimeInSeconds()
{
	struct timespec t1;

	clock_gettime(CLOCK_REALTIME, &t1);

	double d_time = (double)t1.tv_nsec;
	return d_time;
}

class PoseDetectorBase
{
protected:
	double m_beginTimeOfPose;
	double m_durationOfPoseForDetection;
	double m_detectionPercent;

public:

	double GetDetectionPercent() const
	{
		return m_detectionPercent;
	}
	PoseDetectorBase(double durationForDetection = 1)
	{
		m_durationOfPoseForDetection = durationForDetection;
		Reset();
	}
	virtual void SetDurationForDetection(double time)
	{
		m_durationOfPoseForDetection = time;
		Reset();
	}
	virtual PoseDetectionResult checkPose() = 0;
	virtual void Reset()
	{
		m_beginTimeOfPose = -1;
		m_detectionPercent = 0;
	}
	virtual PoseDetectionResult checkPoseDuration()
	{
		double curTime = GetCurrentTimeInSeconds();
		switch(checkPose())
		{
		case IN_POSE_FOR_LITTLE_TIME: //falling through
		case IN_POSE_FOR_LONG_TIME:
			if(m_beginTimeOfPose != -1)
			{
				if(m_durationOfPoseForDetection!=0)
				{
					m_detectionPercent = (curTime - m_beginTimeOfPose) / m_durationOfPoseForDetection;
				} else
				{
					m_detectionPercent = 1;
				}

				if( m_detectionPercent >= 1){
					m_detectionPercent = 1;
					return IN_POSE_FOR_LONG_TIME;
				} 
			} else
			{
				m_beginTimeOfPose = curTime;
			}
			return IN_POSE_FOR_LITTLE_TIME;
		case NOT_ENOUGH_CONFIDENCE:
			if(m_beginTimeOfPose != -1)
			{
				if((curTime - m_beginTimeOfPose) > m_durationOfPoseForDetection){
					//restart waiting
					Reset();
					return IN_POSE_FOR_LITTLE_TIME;
				}
				return IN_POSE_FOR_LITTLE_TIME;
			}
			break;
		case NOT_IN_POSE:
			Reset();
			break;
		}
		return NOT_IN_POSE;
	}
};

class EndPoseDetector: public PoseDetectorBase
{
private:
    nite::UserTrackerFrameRef* m_userTrackerFrame;
    nite::UserId m_nUserId;

public:
    EndPoseDetector(nite::UserTrackerFrameRef* userTrackerFrame, double duration)
        : PoseDetectorBase(duration)
	{
        m_userTrackerFrame = userTrackerFrame;
	}

	void Reset()
	{
		PoseDetectorBase::Reset();
	}

    void SetUserId(nite::UserId nUserId)
	{
		m_nUserId = nUserId;
	}

	PoseDetectionResult checkPose()
	{	
        if (!m_userTrackerFrame->getUserById(m_nUserId)->isVisible())
        {
            return NOT_IN_POSE;
        }

        const nite::Skeleton& skeleton = m_userTrackerFrame->getUserById(m_nUserId)->getSkeleton();
        const nite::SkeletonJoint& leftHand = skeleton.getJoint(nite::JOINT_LEFT_HAND);
        const nite::SkeletonJoint& rightHand = skeleton.getJoint(nite::JOINT_RIGHT_HAND);

        bool bHaveLeftHand = leftHand.getPositionConfidence() >= 0.5;
        bool bHaveRightHand = rightHand.getPositionConfidence() >= 0.5;

        if(!bHaveLeftHand && !bHaveRightHand )
		{
			return NOT_IN_POSE;
		}

		//check for X (left hand is "righter" than right (more than 10 cm)
        float xDist = leftHand.getPosition().x - rightHand.getPosition().x;

		if(xDist < 60 ) return NOT_IN_POSE;

		//check hands to be at same height
        float yDist = fabs(leftHand.getPosition().y - rightHand.getPosition().y);

		if(yDist > 300 ) return NOT_IN_POSE;

		return IN_POSE_FOR_LITTLE_TIME;
	}
};
