#include "RegionDescriptor.h"
#include <opencv2/nonfree/features2d.hpp>
#include <QDebug>
#include <climits>

namespace dai {

RegionDescriptor::RegionDescriptor()
{
}

RegionDescriptor::RegionDescriptor(const InstanceInfo &label, int frameId)
    : Descriptor(label, frameId)
{
}

float RegionDescriptor::distance(const Descriptor& other_desc) const
{
    const RegionDescriptor& other = static_cast<const RegionDescriptor&>(other_desc);

    auto it1 = m_descriptors.constBegin();
    auto it2 = other.m_descriptors.constBegin();
    float distance = 0;

    while (it1 != m_descriptors.constEnd() && it2 != other.m_descriptors.constEnd())
    {
        if ( !(*it1).empty() && !(*it2).empty() )
        {
            cv::BFMatcher matcher;
            //cv::FlannBasedMatcher matcher;
            std::vector<cv::DMatch> matches;
            matcher.match(*it1, *it2, matches);

            double max_dist = 0; double min_dist = std::numeric_limits<double>::max();

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

            if (good_matches == 0) {
                qDebug() << "ZERO DIVISION not managed" << min_dist << max_dist << distance << matches.size();
                throw 1;
            }
        }
        else {
            distance += 1.405f;
        }

        ++it1;
        ++it2;
    }

    if (m_descriptors.empty()) {
        qDebug() <<  "----------------------------------------------";
        qDebug() << "This should not happen! Descriptors are empty";
        qDebug() <<  "----------------------------------------------";
    }

    return distance;
}

bool RegionDescriptor::operator==(const Descriptor& other) const
{
    return true;
}

void RegionDescriptor::addDescriptor(const cv::Mat& descriptor)
{
    m_descriptors << descriptor;
}

} // End Namespace
