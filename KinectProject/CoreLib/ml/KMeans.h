#ifndef KMEANS_H
#define KMEANS_H

#include <QList>
#include <memory>
#include <boost/random.hpp>
#include <QList>
#include <QVector>
#include <QElapsedTimer>

namespace dai {

using namespace std;

// Cluster
template <typename T>
class Cluster {
public:
    shared_ptr<T> centroid;
    QList<shared_ptr<T>> samples;
};

// KMeans
template <typename T>
class KMeans
{
private:
    const QList<shared_ptr<T>> m_samples;
    const int m_k; // Number of clusters
    QList<Cluster<T>> m_clusters; // Clusters with classified samples
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
                kmeans->initialise_centroids();
            }
        }

        return kmeans_best;
    }

    static shared_ptr<KMeans> execute(const QList<shared_ptr<T>> samples, const int k,  const QList<shared_ptr<T>>& centroids)
    {
        shared_ptr<KMeans> kmeans = make_shared<KMeans>(samples, k, centroids);
        kmeans->execute();
        kmeans->compute_compactness();
        return kmeans;
    }

    float compute_compactness()
    {
        float sum = 0;

        for (int i=0; i<m_k; ++i)
        {
            const Cluster<T>& cluster = m_clusters[i];

            for (auto it=cluster.samples.constBegin(); it!=cluster.samples.constEnd(); ++it) {
                sum += (*it)->distance(*(cluster.centroid));
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
            m_clusters << Cluster<T>();

        initialise_centroids();
    }

    KMeans(const QList<shared_ptr<T>> samples, const int k, const QList<shared_ptr<T>>& centroids)
        : m_samples(samples), m_k(k), m_samples_label(samples.size())
    {
        if (k > m_samples.size())
            throw 0;

        for (int i=0; i<m_k; ++i) {
            m_clusters << Cluster<T>();
            m_clusters[i].centroid = centroids[i];
        }
    }

    const QVector<int> getLabels() const
    {
        return m_samples_label;
    }

    const QList<Cluster<T>>& getClusters() const
    {
        return m_clusters;
    }

    const QList<shared_ptr<T>> getSamples() const
    {
        return m_samples;
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

    void initialise_centroids()
    {
        boost::random::mt19937 gen;
        gen.seed(time(0));
        boost::random::uniform_int_distribution<> dist(0, m_samples.size() - 1);
        boost::variate_generator<boost::mt19937&, boost::random::uniform_int_distribution<>> uniform_rnd(gen, dist);

        reset();

        int i=0;

        while (i<m_k)
        {
            m_clusters[i].centroid = m_samples[uniform_rnd()]; // Asign the pointer
            bool exists = false;

            for (int j=0; j<i && !exists; ++j) {
                if (*(m_clusters[i].centroid) == *(m_clusters[j].centroid)) {
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

        for (auto it = m_clusters.begin(); it != m_clusters.end(); ++it) {
            (*it).samples.clear();
            (*it).centroid = nullptr;
        }
    }

    void execute()
    {
        int num_changes = 0;

        do {
            num_changes = kmeans_clustering();

            if (num_changes > 0) {
                recalculate_centroids();
            }
        }
        while (num_changes > 0);
    }

    int kmeans_clustering()
    {
        int num_changes = 0;

        // Empty clusters
        for (int i=0; i<m_k; ++i)
            m_clusters[i].samples.clear();

        // Create new clusters
        int i = 0;

        for (auto sample = m_samples.constBegin(); sample != m_samples.constEnd(); ++sample)
        {
            int cluster_id = searchClosestCluster(*sample);

            // Add to the cluster
            m_clusters[cluster_id].samples << *sample;

            // Count changes
            if (m_samples_label[i] != cluster_id) {
                m_samples_label[i] = cluster_id;
                num_changes++;
            }

            ++i;
        }

        return num_changes;
    }

    // Recalculate centroids using the item of each cluster that minimices its distance
    // to the rest of the items of the cluster.
    void recalculate_centroids()
    {
        for (int i=0; i<m_k; ++i) {
            shared_ptr<T> item = minItem(i);
            if (item) {
                m_clusters[i].centroid = item;
            }
            // else cluster is empty (but is not an error)
        }
    }

    shared_ptr<T> minItem(int cluster_id) const
    {
        QElapsedTimer timer;
        timer.start();
        const QList<shared_ptr<T>>& items = m_clusters[cluster_id].samples;
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

        qDebug() << "Min.Item (ms)" << timer.elapsed();

        return minItem;
    }

    int searchClosestCluster(shared_ptr<T> sample)
    {
        float min_distance = std::numeric_limits<float>::max();
        int closest_cluster = -1;

        for (int i=0; i<m_k; ++i)
        {
            shared_ptr<T> centroid = m_clusters[i].centroid;

            if (centroid == nullptr) {
                qDebug() << "Super error 3";
                throw 3;
            }

            float distance = sample->distance(*centroid);

            if (distance < min_distance) {
                min_distance = distance;
                closest_cluster = i;
            }
        }

        if (closest_cluster == -1) {
            qDebug() << "Super error 4";
            throw 3;
        }

        return closest_cluster;
    }

};

} // End Namespace

#endif // KMEANS_H
