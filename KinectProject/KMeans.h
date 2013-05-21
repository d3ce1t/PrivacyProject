#ifndef KMEANS_H
#define KMEANS_H

#include <QList>

namespace dai {

class KMeans
{
public:

    static const KMeans *execute(const float* sample, const int n, const int k, int times = 5);

    virtual ~KMeans();
    float *getClusterMask() const;
    const QList<float>* getClusterValues() const;
    const float* getSamples() const;
    const QList<float> &getCentroids() const;
    int getNumSamples() const;
    int getK() const;
    float getCompactness() const;

private:
    static float distance(float v1, float v2);

    // Private constructor
    KMeans(const float *sample, const int n, const int k);
    //KMeans(const float *sample, const int dim, const int n, const int k);

    // Private Methods
    void reset();
    void initialise();
    void execute();
    void load_cluster_values();
    float compute_compactness();
    int kmeans_clustering();
    void kmeans_recalculate_centroids();


    // Class atributes
    const float* m_sample;
    const int m_dim;
    const int m_n;
    const int m_k;
    QList<float> m_centroid;
    float* m_clusters_mask;
    QList<float>* m_clusters_list; // List vector
    float m_compactness;
};

} // End Namespace

#endif // KMEANS_H
