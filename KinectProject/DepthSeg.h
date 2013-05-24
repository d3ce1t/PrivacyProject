#ifndef DEPTHSEG_H
#define DEPTHSEG_H

#include "types/DepthFrame.h"

namespace dai {

class DepthSeg
{
public:
    DepthSeg(const DepthFrame& frame);
    virtual ~DepthSeg();
    void execute();
    float *getClusterMask() const;
    float getCluster(int row, int column) const;
    void print_cluster_mask(int row, int column) const;

private:
    struct NeighbourOffset {
        int x;
        int y;
    };

    const static NeighbourOffset neighbour_offset[8];

    bool is_continuous(int row1, int col1, int row2, int col2) const;
    void setCluster(int row, int column, float value);
    void load_neighbourhood(int row, int column, float neighbourhood[8]) const;
    int num_assigned_neighbourhood(int row, int column) const;
    QString print_decimal(int number) const;
    QList<float> get_neighbourhood_clusters(int row, int column) const;
    void merge_clusters(int row, int column, float src_cluster, float merge_cluster);

    /**
     * Returns the cluster whose size is bigger than others
     * @brief highest_neighbour_cluster
     * @param neighbour_clusters
     * @return
     */
    float highest_neighbour_cluster(const QList<float>& neighbour_clusters) const;
    float min_neighbour_cluster(const QList<float>& neighbour_clusters) const;

    const DepthFrame& m_frame;
    float* m_clusters_mask;
    QList<int> m_clusters_size;
};

} // End Namespace

#endif // DEPTHSEG_H
