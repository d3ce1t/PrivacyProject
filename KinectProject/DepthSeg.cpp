#include "DepthSeg.h"
#include <QDebug>
#include <iostream>
#include <limits>
#include <cmath>

using namespace std;

namespace dai {

const DepthSeg::NeighbourOffset DepthSeg::neighbour_offset[8] = {
    {-1, -1},
    {0, -1},
    {+1, -1},
    {+1, 0},
    {+1, +1},
    {0, +1},
    {-1, +1},
    {-1, 0}
};

DepthSeg::DepthSeg(const DepthFrame &frame)
    : m_frame(frame)
{
    m_clusters_mask = new float[frame.getHeight() * frame.getWidth()];

    for (int i=0; i<frame.getHeight()*frame.getWidth(); ++i)
    {
        m_clusters_mask[i] = -1;
    }
}

DepthSeg::~DepthSeg()
{
    delete[] m_clusters_mask;
    m_clusters_mask = NULL;
}

float* DepthSeg::getClusterMask() const
{
    return m_clusters_mask;
}

void DepthSeg::execute()
{
    int cluster_index = 0;

    for (int row=1; row<m_frame.getHeight()-1; ++row)
    {
        for (int column=1; column<m_frame.getWidth()-1; ++column)
        {
            float currentItem = m_frame.getItem(row, column);
            float neighbourhood[8];
            load_neighbourhood(row, column, neighbourhood);
            QList<float> different_clusters = get_neighbourhood_clusters(row, column);

            if (currentItem != 0)
            {
                // Determine my cluster
                float current_cluster = getCluster(row, column);

                // Not assigned cluster neither neighbourhood has assigned cluster
                // Create a new cluster
                if (current_cluster == -1 && different_clusters.size() == 0)
                {
                    current_cluster = cluster_index;
                    setCluster(row, column, current_cluster);
                    m_clusters_size.append(1);
                    cluster_index++;
                }
                // Not assigned cluster an has one or more neighbour
                // My cluster is the cluster of my neighbour whose cluster size is the highest
                else if (current_cluster == -1 && num_assigned_neighbourhood(row, column) >= 1)
                {
                    current_cluster = highest_neighbour_cluster(different_clusters);
                    setCluster(row, column, current_cluster);
                    m_clusters_size[current_cluster]++;
                }
                // Assigned cluster
                else if (current_cluster != -1)
                {
                    float root_cluster = min_neighbour_cluster(different_clusters);

                    if (current_cluster != root_cluster) {
                        //print_cluster_mask(row, column);
                        merge_clusters(row, column, root_cluster, current_cluster);
                        current_cluster = root_cluster;
                    }
                }

                // Propagate selection to my neighbourhood
                /*for (int i=0; i<8; ++i)
                {
                    int neighbour_row = row + neighbour_offset[i].y;
                    int neighbour_column = column + neighbour_offset[i].x;

                    if (is_continuous(row, column, neighbour_row, neighbour_column) && getCluster(neighbour_row, neighbour_column) == -1)
                    {
                        setCluster(neighbour_row, neighbour_column, current_cluster);

                        if (current_cluster == -1) {
                            print_cluster_mask(row, column);
                            QList<float> different = get_neighbourhood_clusters(row, column);
                            qDebug() << different;
                        }

                        m_clusters_size[current_cluster]++;
                    }
                }*/
            }
        }
    }

    //qDebug() << "Clusters size" << m_clusters_size.size() << m_clusters_size;
}

bool DepthSeg::is_continuous(int row1, int col1, int row2, int col2) const
{
    bool  result = false;
    float value1 = m_frame.getItem(row1, col1);
    float value2 = m_frame.getItem(row2, col2);

    if (value1 != 0 && value2 != 0 && fabs(value1 - value2) == 0) {

        for (int i=0; i<8 && !result; ++i) {
            if (row2 == row1 + neighbour_offset[i].y && col2 == col1 +neighbour_offset[i].x)
                result = true;
        }
    }

    return result;
}

void DepthSeg::merge_clusters(int row, int column, float src_cluster, float merge_cluster)
{
    for (int i=0; i<=row; i++)
    {
        for (int j=0; j<m_frame.getWidth(); ++j)
        {
            float current_cluster = getCluster(i, j);

            if (current_cluster != -1 && current_cluster == merge_cluster)
            {
                m_clusters_size[current_cluster]--;
                m_clusters_size[src_cluster]++;

                if (m_clusters_size[current_cluster] < 0 ||
                        m_clusters_size[src_cluster] <0 ) {
                    qDebug() << "weha";
                }

                setCluster(i, j, src_cluster);
            }
        }
    }
}

float DepthSeg::highest_neighbour_cluster(const QList<float>& neighbour_clusters) const
{
    int highest_cluster_size = -1;
    int bigger_cluster = -1;

    for (int i=0; i<neighbour_clusters.size(); ++i)
    {
        int cluster = neighbour_clusters[i];
        int cluster_size = m_clusters_size[cluster];

        if (cluster_size > highest_cluster_size) {
            highest_cluster_size = cluster_size;
            bigger_cluster = cluster;
        }
    }

    Q_ASSERT(bigger_cluster != -1);

    return bigger_cluster;
}

float DepthSeg::min_neighbour_cluster(const QList<float>& neighbour_clusters) const
{
    int min_cluster =  std::numeric_limits<int>::max();

    for (int i=0; i<neighbour_clusters.size(); ++i)
    {
        int cluster = neighbour_clusters[i];

        if (cluster < min_cluster) {
            min_cluster = cluster;
        }
    }

    Q_ASSERT(min_cluster != -1);

    return min_cluster;
}

void DepthSeg::print_cluster_mask(int row, int column) const
{
    int margin = 1;
    for (int i=row-margin; i<=row+margin; ++i) {
        for (int j=column-margin; j<=column+margin; ++j) {
            cout << print_decimal(getCluster(i, j) + 1).toStdString() << " ";
        }
        cout << endl;
    }
}

QString DepthSeg::print_decimal(int number) const
{
    QString cadena = QString::number(number);

    if (number < 10) {
        cadena = "0" + cadena;
    }

    return cadena;
}

void DepthSeg::load_neighbourhood(int row, int column, float neighbourhood[8]) const
{
    for (int i=0; i<8; ++i) {
        neighbourhood[i] = m_frame.getItem(row + neighbour_offset[i].y, column + neighbour_offset[i].x);
    }
}

QList<float> DepthSeg::get_neighbourhood_clusters(int row, int column) const
{
    QList<float> result;

    for (int i=0; i<8; ++i) {
        float cluster = getCluster(row + neighbour_offset[i].y, column + neighbour_offset[i].x);
        if (cluster != -1 && !result.contains(cluster))
            result.append(cluster);
    }

    return result;
}

int DepthSeg::num_assigned_neighbourhood(int row, int column) const
{
    int total = 0;

    for (int i=0; i<8; ++i) {
        if (getCluster(row + neighbour_offset[i].y, column + neighbour_offset[i].x) != -1)
            total++;
    }

    return total;
}

float DepthSeg::getCluster(int row, int column) const
{
    if (row < 0 || row >= m_frame.getHeight() || column < 0 || column >= m_frame.getWidth() )
        throw 1;

    return m_clusters_mask[row * m_frame.getWidth() + column];
}

void DepthSeg::setCluster(int row, int column, float value)
{
    if (row < 0 || row >= m_frame.getHeight() || column < 0 || column >= m_frame.getWidth() )
        throw 1;

    m_clusters_mask[row * m_frame.getWidth() + column] = value;
}

} // End Namespace
