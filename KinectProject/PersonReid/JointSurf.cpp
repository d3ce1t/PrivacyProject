#include "JointSurf.h"
#include <opencv2/nonfree/features2d.hpp>
#include <QDebug>

namespace dai {

JointSurf::JointSurf()
{
}

JointSurf::JointSurf(const InstanceInfo &label, int frameId)
    : Descriptor(label, frameId)
{
}

float JointSurf::distance(const Descriptor& other_desc) const
{
    const JointSurf& other = static_cast<const JointSurf&>(other_desc);

    auto it1 = m_descriptors.constBegin();
    auto it2 = other.m_descriptors.constBegin();
    float distance = 0;

    while (it1 != m_descriptors.constEnd() && it2 != other.m_descriptors.constEnd())
    {
        if ( !(*it1).empty() && !(*it2).empty() )
        {
            cv::FlannBasedMatcher matcher;
            std::vector<cv::DMatch> matches;
            matcher.match(*it1, *it2, matches);

            double max_dist = 0; double min_dist = 100;

            //-- Quick calculation of max and min distances between keypoints
            for( uint i = 0; i < matches.size(); i++ ) {
                double dist = matches[i].distance;
                if( dist < min_dist ) min_dist = dist;
                if( dist > max_dist ) max_dist = dist;
            }

            float local_distance = 0;
            int good_matches = 0;

            for( uint i = 0; i < matches.size(); i++ )
            {
                if(matches[i].distance <= cv::max(2*min_dist, 0.02)) {
                    local_distance += matches[i].distance;
                    good_matches++;
                }
            }

            distance += local_distance / good_matches;
            //qDebug() << "Src.Points" << (*it1).rows << "Dst.Points" << (*it2).rows << "Matches" << matches.size();
        }
        else {
            distance += 1.40513f;
        }

        ++it1;
        ++it2;
    }

    return distance;
}

bool JointSurf::operator==(const Descriptor& other) const
{
    return true;
}

void JointSurf::addSurfDescriptor(const cv::Mat& descriptor)
{
    m_descriptors << descriptor;
}

} // End Namespace
