#include "KMeans.h"
#include "Utils.h"
#include <cmath>
#include <limits>
#include <QDebug>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <iostream>

namespace dai {

KMeans::KMeans(const float* sample, const int n, const int k)
    : m_sample(sample), m_n(n), m_k(k), m_dim(1)
{
    if (k > n)
        throw 0;

    m_clusters_mask = new float[n];
    m_clusters_list = new QList<float>[m_k];
    initialise();
}

/*KMeans::KMeans(const float *sample, const int dim, const int n, const int k)
    : m_sample(sample), m_dim(dim), m_n(n), m_k(k)
{
    if (k > n)
        throw 0;

    m_clusters_mask = new float[n];
    m_clusters_list = new QList<float>[m_k];
    initialise();
}*/

KMeans::~KMeans()
{
    m_centroid.clear();

    if (m_clusters_mask) {
        delete[] m_clusters_mask;
        m_clusters_mask = nullptr;
    }

    if (m_clusters_list) {
        delete[] m_clusters_list;
        m_clusters_list = nullptr;
    }
}

float KMeans::compute_compactness()
{
    load_cluster_values();
    float sum = 0;

    for (int i=0; i<m_k; ++i)
    {
        QList<float>& x_samples = m_clusters_list[i];

        for (int j=0; j<x_samples.size(); ++j)
        {
            sum += fabs(x_samples[j] - m_centroid[i]);
        }
    }

    m_compactness = sum;

    return sum;
}

void KMeans::reset()
{
    for (int i=0; i<m_n; ++i)
        m_clusters_mask[i] = -1;

    m_centroid.clear();

    for (int i=0; i<m_k; ++i) {
        m_centroid.append(0);
        m_clusters_list[i].clear();
    }
}

void KMeans::execute()
{
    int num_changes = 0;

    do {
        num_changes = kmeans_clustering();

        if (num_changes > 0) {
            kmeans_recalculate_centroids();
        }
    } while (num_changes > 0);
}

void KMeans::initialise()
{
    //Mersenne Twister pseudorandomnumber generator
    boost::random::mt19937 gen;
    gen.seed(time(0));
    boost::random::uniform_int_distribution<> dist(0, m_n-1);

    reset();
    int i=0;

    while (i<m_k) {
        m_centroid[i] = m_sample[dist(gen)];
        bool exists = false;

        for (int j=0; j<i && !exists; ++j) {
            if (m_centroid[i] == m_centroid[j]) {
                exists = true;
            }
        }

        if (!exists) {
            i++;
        }
    }

    qSort(m_centroid);
}

int KMeans::kmeans_clustering()
{
    int num_changes = 0;

    for (int i=0; i<m_n; ++i)
    {
        float min_distance = std::numeric_limits<float>::max();
        int cluster_index = -1;

        for (int j=0; j<m_k; ++j)
        {
            float distance = KMeans::distance(m_sample[i], m_centroid[j]);

            if (distance < min_distance) {
                min_distance = distance;
                cluster_index = j;
            }
        }

        if (m_clusters_mask[i] != cluster_index) {
            num_changes++;
        }

        m_clusters_mask[i] = cluster_index;
    }

    return num_changes;
}

void KMeans::kmeans_recalculate_centroids()
{
    load_cluster_values();

    for (int i=0; i<m_k; ++i) {
        m_centroid[i] = m_clusters_list[i].isEmpty() ? 0 : dai::avg(m_clusters_list[i]);
    }
}

void KMeans::load_cluster_values()
{
    for (int i=0; i<m_k; ++i)
        m_clusters_list[i].clear();

    for (int i=0; i<m_n; ++i) {
        int cluster_index = m_clusters_mask[i];
        m_clusters_list[cluster_index] << m_sample[i];
    }
}

//
// Getters
//
float* KMeans::getClusterMask() const
{
    return m_clusters_mask;
}

const QList<float>* KMeans::getClusterValues() const
{
    return m_clusters_list;
}

const float* KMeans::getSamples() const
{
    return m_sample;
}

const QList<float>& KMeans::getCentroids() const
{
    return m_centroid;
}

int KMeans::getNumSamples() const
{
    return m_n;
}

int KMeans::getK() const
{
    return m_k;
}

float KMeans::getCompactness() const
{
    return m_compactness;
}

//
// Static methods
//

const KMeans* KMeans::execute(const float *sample, const int n, const int k, int times)
{
    if (times <= 0)
        throw 1;

    KMeans* kmeans = nullptr;
    KMeans* kmeans_best = nullptr;
    float min_compactness = std::numeric_limits<float>::max();
    float compactness = 0;

    for (int i=0; i<times; ++i)
    {
        if (kmeans == nullptr)
            kmeans = new KMeans(sample, n, k);

        kmeans->execute();
        compactness = kmeans->compute_compactness();

        if (compactness < min_compactness) {
            min_compactness = compactness;
            if (kmeans_best != nullptr)
                delete kmeans_best;
            kmeans_best = kmeans;
            kmeans = nullptr;
        } else {
            kmeans->initialise();
        }
    }

    if (kmeans != nullptr) {
        delete kmeans;
        kmeans = nullptr;
    }

    for (int i=0; i<kmeans_best->m_k; ++i) {
        qSort(kmeans_best->m_clusters_list[i]);
    }

    return kmeans_best;
}


float KMeans::distance(float v1, float v2) {
    return fabs(v1 - v2);
}

} // End Namespace
