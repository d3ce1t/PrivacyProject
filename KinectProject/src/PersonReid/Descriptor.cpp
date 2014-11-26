#include "Descriptor.h"
#include <QtConcurrent>
#include <QElapsedTimer>
#include <climits>

namespace dai {

Descriptor::Descriptor()
{
}

Descriptor::Descriptor(const InstanceInfo& label, int frameId)
{
    m_label = label;
    m_frameId = frameId;
}

bool Descriptor::operator==(const Descriptor& other) const
{
    return m_label == other.m_label && m_frameId == other.m_frameId;
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
                //m_min_feature = intermediate.second;
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
