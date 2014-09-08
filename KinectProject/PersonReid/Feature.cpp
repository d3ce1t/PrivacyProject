#include "Feature.h"
#include <climits>
#include <QtConcurrent>
#include <QElapsedTimer>
#include <QDebug>


namespace dai {

shared_ptr<Feature> Feature::minFeatureParallel(const QList<shared_ptr<Feature>> &features)
{
    struct AddDistance
    {
        AddDistance(QList<shared_ptr<Feature>> list)
        : feature_list(list) { }

        typedef FeatureWrapper result_type;

        FeatureWrapper operator()(const shared_ptr<Feature> &feature)
        {
            float distance = 0;

            for (auto it = feature_list.constBegin(); it != feature_list.constEnd(); ++it)
            {
                distance += feature->distance(*(*it));
            }

            return {distance, feature};
        }

        QList<shared_ptr<Feature>> feature_list;
    };

    struct AverageDistance
    {
        AverageDistance()
        {
            min_distance = std::numeric_limits<float>::max();
        }

        void operator()(shared_ptr<Feature>& result, const FeatureWrapper &intermediate)
        {
            if (intermediate.distance < min_distance) {
                min_distance = intermediate.distance;
                result = intermediate.feature;
                //m_min_feature = intermediate.second;
            }
        }

        float min_distance;
    };

    qDebug() << "Min feature...";
    QElapsedTimer timer;
    timer.start();
    shared_ptr<Feature> result = QtConcurrent::blockingMappedReduced
            <shared_ptr<Feature>, QList<shared_ptr<Feature>>, AddDistance, AverageDistance>
            (features,AddDistance(features),AverageDistance());
    qDebug() << "Time" << timer.elapsed();

    return result;
}

shared_ptr<Feature> Feature::minFeature(const QList<shared_ptr<Feature>>& features)
{
    qDebug() << "Min feature...";
    QElapsedTimer timer;
    timer.start();
    float min_distance = std::numeric_limits<float>::max();
    shared_ptr<Feature> selectedFeature;

    foreach (shared_ptr<Feature> feature1, features)
    {
        float distance = 0;

        foreach (shared_ptr<Feature> feature2, features)
        {
            distance += feature1->distance(*feature2);
        }

        if (distance < min_distance) {
            min_distance = distance;
            selectedFeature = feature1;
        }
    }

    qDebug() << "Time" << timer.elapsed();

    return selectedFeature;
}


Feature::Feature(InstanceInfo label, int frameId)
{
    m_label = label;
    m_frameId = frameId;
}

void Feature::addHistogram(const Histogram2c &hist)
{
    m_histograms.append(hist); // copy
}

bool Feature::operator==(const Feature& other) const
{
    if (m_histograms.size() == other.m_histograms.size())
        return false;

    bool equal = true;
    auto it1 = m_histograms.constBegin();
    auto it2 = other.m_histograms.constBegin();

    while (equal && it1 != m_histograms.constEnd() && it2 != other.m_histograms.constEnd())
    {
        equal = *it1 == *it2;
        ++it1;
        ++it2;
    }

    return equal;
}

float Feature::distance(const Feature& other) const
{
    if (m_histograms.size() != other.m_histograms.size())
        return 1.0f;

    auto it1 = m_histograms.constBegin();
    auto it2 = other.m_histograms.constBegin();

    double distance = 0;

    while (it1 != m_histograms.constEnd() && it2 != other.m_histograms.constEnd())
    {
        distance += (*it1).distance(*it2);
        ++it1;
        ++it2;
    }

    return distance / m_histograms.size();
}

 const InstanceInfo& Feature::label() const
 {
     return m_label;
 }

} // End Namespace
