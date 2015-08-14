#include "Descriptor.h"
#include <QtConcurrent>
#include <QElapsedTimer>
#include <climits>

namespace dai {

/*Descriptor::Descriptor()
{
}*/

Descriptor::Descriptor(const InstanceInfo& label, int frameId)
{
    m_label = label;
    m_frameId = frameId;
}

bool Descriptor::operator==(const Descriptor& other) const
{
    return m_label == other.m_label && m_frameId == other.m_frameId;
}

float Descriptor::minDistanceParallel(const DescriptorPtr feature, const QList<DescriptorPtr>& samples)
{
    struct AddDistance
    {
        AddDistance(DescriptorPtr feature)
        : m_feature(feature) { }

        typedef float result_type;

        float operator()(const DescriptorPtr& sample)
        {
            return m_feature->distance(*sample);
        }

        DescriptorPtr m_feature;
    };

    struct AverageDistance
    {
        AverageDistance()
        {
            min_distance = std::numeric_limits<float>::max();
        }

        void operator()(float& result, const float &intermediate)
        {
            if (intermediate < min_distance) {
                min_distance = intermediate;
                result = min_distance;
            }
        }

        float min_distance;
    };


    float result = QtConcurrent::blockingMappedReduced
            <float, QList<DescriptorPtr>, AddDistance, AverageDistance>
            (samples,AddDistance(feature),AverageDistance());


    return result;
}

DescriptorPtr Descriptor::minFeatureParallel(const QList<DescriptorPtr> &features)
{
    struct FeatureWrapper
    {
        float distance;
        DescriptorPtr feature;
    };

    struct AddDistance
    {
        AddDistance(QList<DescriptorPtr> list)
        : feature_list(list) { }

        typedef FeatureWrapper result_type;

        FeatureWrapper operator()(const DescriptorPtr &feature)
        {
            float distance = 0;

            for (auto it = feature_list.constBegin(); it != feature_list.constEnd(); ++it)
            {
                distance += feature->distance(*(*it));
            }

            return {distance, feature};
        }

        QList<DescriptorPtr> feature_list;
    };

    struct AverageDistance
    {
        AverageDistance()
        {
            min_distance = std::numeric_limits<float>::max();
        }

        void operator()(DescriptorPtr& result, const FeatureWrapper &intermediate)
        {
            if (intermediate.distance < min_distance) {
                min_distance = intermediate.distance;
                result = intermediate.feature;
            }
        }

        float min_distance;
    };

    qDebug() << "Min feature...";
    QElapsedTimer timer;
    timer.start();
    DescriptorPtr result = QtConcurrent::blockingMappedReduced
            <DescriptorPtr, QList<DescriptorPtr>, AddDistance, AverageDistance>
            (features,AddDistance(features),AverageDistance());
    qDebug() << "Time" << timer.elapsed();

    return result;
}

DescriptorPtr Descriptor::minFeature(const QList<DescriptorPtr>& features)
{
    qDebug() << "Min feature...";
    QElapsedTimer timer;
    timer.start();
    float min_distance = std::numeric_limits<float>::max();
    DescriptorPtr selectedFeature;

    for (DescriptorPtr feature1 : features)
    {
        float distance = 0;

        for (DescriptorPtr feature2 : features)
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

} // End Namespace
