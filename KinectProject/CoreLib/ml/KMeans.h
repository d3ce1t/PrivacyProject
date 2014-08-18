#ifndef KMEANS_H
#define KMEANS_H

#include <QList>
#include <memory>
#include <boost/random.hpp>

namespace dai {

using namespace std;

template <typename T>
class KMeans
{
    const QList<shared_ptr<T>> m_samples;
    const int m_k; // Number of clusters
    QList<shared_ptr<T>> m_centroid; // Centroids
    QList<QList<shared_ptr<T>>> m_clusters; // Clusters with classified samples
    QVector<int> m_samples_label;
    float m_compactness;

public:

    static shared_ptr<KMeans> execute(const QList<shared_ptr<T>> samples, const int k, int times = 5)
    {
        if (times <= 0)
            throw 1;

        shared_ptr<KMeans> kmeans = nullptr;
        shared_ptr<KMeans> kmeans_best = nullptr;
        float min_compactness = std::numeric_limits<float>::max();

        for (int i=0; i<times; ++i)
        {
            if (kmeans == nullptr)
                kmeans = make_shared<KMeans>(samples, k);

            kmeans->execute();

            float compactness = kmeans->compute_compactness();

            if (compactness < min_compactness) {
                min_compactness = compactness;
                kmeans_best = kmeans;
                kmeans = nullptr;
            } else {
                kmeans->initialise();
            }
        }

        return kmeans_best;
    }

    float compute_compactness()
    {
        feed_clusters();

        float sum = 0;

        for (int i=0; i<m_k; ++i)
        {
            QList<shared_ptr<T>>& cluster_samples = m_clusters[i];

            for (auto it=cluster_samples.constBegin(); it!=cluster_samples.constEnd(); ++it) {
                sum += (*it)->distance(*m_centroid[i]);
            }
        }

        m_compactness = sum;

        return sum;
    }

    KMeans(const QList<shared_ptr<T>> samples, const int k)
        : m_samples(samples), m_k(k), m_samples_label(samples.size())
    {
        if (k > m_samples.size())
            throw 0;

        for (int i=0; i<m_k; ++i)
            m_clusters << QList<shared_ptr<T>>();

        initialise();
    }

    virtual ~KMeans()
    {
        m_centroid.clear();
    }

    const QVector<int> getLabels() const
    {
        return m_samples_label;
    }

    const QList<QList<shared_ptr<T>>> getClusters() const
    {
        return m_clusters;
    }

    const QList<shared_ptr<T>> getSamples() const
    {
        return m_samples;
    }

    const QList<shared_ptr<T>> getCentroids() const
    {
        return m_centroid;
    }

    int getNumSamples() const
    {
        return m_samples.size();
    }

    int getK() const
    {
        return m_k;
    }

    float getCompactness() const
    {
        return m_compactness;
    }

private:

    void initialise()
    {
        boost::random::mt19937 gen;
        gen.seed(time(0));
        boost::random::uniform_int_distribution<> dist(0, m_samples.size() - 1);
        boost::variate_generator<boost::mt19937&, boost::random::uniform_int_distribution<>> uniform_rnd(gen, dist);

        reset();

        int i=0;

        while (i<m_k)
        {
            m_centroid[i] = m_samples[uniform_rnd()]; // Asign the pointer
            bool exists = false;

            for (int j=0; j<i && !exists; ++j) {
                if (*m_centroid[i] == *m_centroid[j]) {
                    exists = true;
                }
            }

            if (!exists)
                i++;
        }
    }

    void reset()
    {
        for (auto item = m_samples_label.begin(); item != m_samples_label.end(); ++item)
            *item = -1;

        m_centroid.clear();

        for (auto it = m_clusters.begin(); it != m_clusters.end(); ++it) {
            (*it).clear();
            m_centroid << nullptr;
        }
    }

    void execute()
    {
        int num_changes = 0;

        do {
            num_changes = kmeans_clustering();

            if (num_changes > 0) {
                kmeans_recalculate_centroids();
            }
        }
        while (num_changes > 0);
    }

    int kmeans_clustering()
    {
        int num_changes = 0;

        int i = 0;

        for (auto sample = m_samples.constBegin(); sample != m_samples.constEnd(); ++sample)
        {
            float min_distance = std::numeric_limits<float>::max();
            int cluster_index = -1;

            for (int j=0; j<m_k; ++j)
            {
                shared_ptr<T> centroid = m_centroid[j];

                if (centroid == nullptr) {
                    qDebug() << "Super error 3";
                    throw 3;
                }

                float distance = (*sample)->distance(*centroid);

                if (distance < min_distance) {
                    min_distance = distance;
                    cluster_index = j;
                }

                if (cluster_index == -1) {
                    qDebug() << "Super error 4" << "distance" << distance;
                    throw 3;
                }
            }

            if (m_samples_label[i] != cluster_index) {
                m_samples_label[i] = cluster_index;
                num_changes++;
            }

            ++i;
        }

        return num_changes;
    }

    // Recalculate centroids using the item of each cluster that minimices its distance
    // to the rest of the items of the cluster.
    void kmeans_recalculate_centroids()
    {
        feed_clusters();

        for (int i=0; i<m_k; ++i) {
            shared_ptr<T> item = minItem(i);
            if (item)
                m_centroid[i] = item;
            // else cluster is empty (but is not an error)
        }
    }

    shared_ptr<T> minItem(int cluster_id) const
    {
        const QList<shared_ptr<T>>& items = m_clusters[cluster_id];
        float minDistance = std::numeric_limits<float>::max();
        shared_ptr<T> minItem = nullptr;
        float sum = 0;

        for (auto it1 = items.constBegin(); it1 != items.constEnd(); ++it1)
        {
            sum = 0;

            for (auto it2 = items.constBegin(); it2 != items.constEnd(); ++it2) {
                sum += (*it1)->distance(*(*it2));
            }

            if (sum < minDistance) {
                minDistance = sum;
                minItem = (*it1);
            }
        }

        return minItem;
    }

    void feed_clusters()
    {
        for (int i=0; i<m_k; ++i)
            m_clusters[i].clear();

        for (int i=0; i<m_samples.size(); ++i)
        {
            int cluster_index = m_samples_label[i];

            if (cluster_index == -1) {
                qDebug() << "Super error 1";
                throw 3;
            } else if (cluster_index >= m_k) {
                qDebug() << "Super error 2";
                throw 3;
            }

            m_clusters[cluster_index] << m_samples[i];
        }
    }

};

} // End Namespace

#endif // KMEANS_H
