#ifndef PERSONREID_H
#define PERSONREID_H

#include <QObject>
#include "dataset/Dataset.h"
#include "Descriptor.h"
#include "types/ColorFrame.h"
#include "types/MaskFrame.h"
#include "types/SkeletonFrame.h"
#include "openni/OpenNIDevice.h"
#include "opencv2/features2d/features2d.hpp"

namespace dai {

class PersonReid : public QObject
{
    Q_OBJECT

public:
    // Training and Testing
    QMultiMap<int, DescriptorPtr> train(Dataset *dataset, QList<int> actors, int camera);
    void validate(Dataset* dataset, const QList<int> &actors, int camera, const QMultiMap<int, DescriptorPtr>& gallery, QVector<float>& results, int *num_tests);

    // Features
    DescriptorPtr computeFeature(Dataset *dataset, shared_ptr<InstanceInfo> instance_info);

    DescriptorPtr feature_2parts_hist(shared_ptr<ColorFrame> colorFrame, const InstanceInfo& instance_info) const;

    DescriptorPtr feature_fusion(ColorFrame &colorFrame, DepthFrame &depthFrame, MaskFrame &maskFrame,
                                            Skeleton &skeleton, const InstanceInfo& instance_info) const;

    DescriptorPtr feature_global_hist(ColorFrame &colorFrame, MaskFrame &maskFrame, const InstanceInfo& instance_info) const;

    DescriptorPtr feature_joints_hist(ColorFrame &colorFrame, DepthFrame &depthFrame, MaskFrame &maskFrame,
                                            Skeleton &skeleton, const InstanceInfo& instance_info) const;

    DescriptorPtr feature_region_descriptor(ColorFrame &colorFrame, DepthFrame &depthFrame, MaskFrame &maskFrame,
                                            Skeleton &skeleton, const InstanceInfo& instance_info) const;

    DescriptorPtr feature_pointinterest_descriptor(ColorFrame &colorFrame, MaskFrame &maskFrame, const InstanceInfo& instance_info) const;

    DescriptorPtr feature_joint_descriptor(ColorFrame &colorFrame, DepthFrame &depthFrame, MaskFrame &maskFrame, Skeleton &skeleton, const InstanceInfo& instance_info) const;


    DescriptorPtr feature_skeleton_distances(ColorFrame &colorFrame, Skeleton &skeleton, const InstanceInfo& instance_info) const;

    // Utils
    static void makeUpJoints(Skeleton& skeleton, bool only_middle_points = false);
    static void makeUpOnlySomeJoints(Skeleton& skeleton);
    cv::KeyPoint createKeyPoint(const SkeletonJoint& joint, const Skeleton &skeleton, shared_ptr<MaskFrame> voronoi) const;
    static SkeletonJoint getCloserJoint(const Point3f& cloudPoint, const QList<SkeletonJoint>& joints);
    static shared_ptr<MaskFrame> getVoronoiCells(const DepthFrame& depthFrame, const MaskFrame& maskFrame, const Skeleton& skeleton);
    static shared_ptr<MaskFrame> getVoronoiCellsParallel(const DepthFrame& depthFrame, const MaskFrame& maskFrame, const Skeleton& skeleton);
    static void colorImageWithVoronoid(ColorFrame &colorFrame, MaskFrame &voronoi);
    void highLightMask(ColorFrame &colorFrame, MaskFrame &maskFrame) const;
    void highLightDepth(ColorFrame &colorFrame, DepthFrame &depthFrame) const;
    QHashDataFrames allocateMemory() const;
    QMap<float, int> compute_distances_to_all_samples(const Descriptor& query, const QList<DescriptorPtr>& gallery);
    int cummulative_match_curve(QMap<float, int> &query_results, QVector<float>& results, int label);
    void normalise_results(QVector<float> &results, int num_accumulated_samples) const;
    void print_results(const QVector<float>& results) const;
    void print_query_results(const QMap<float, int>& query_results, int pos) const;

public slots:
    void execute();

private:
    static RGBColor _colors[20];

    void show_images(shared_ptr<ColorFrame> colorFrame, shared_ptr<MaskFrame> maskFrame, shared_ptr<DepthFrame> depthFrame, shared_ptr<Skeleton> skeleton);
    //void printClusters(const QList<Cluster<Descriptor> > &clusters) const;
    void drawSkeleton(ColorFrame &colorFrame, const Skeleton &skeleton) const;
    void drawPoint(ColorFrame &colorFrame, int x, int y, RGBColor color = {255, 0, 0}) const;

    OpenNIDevice* m_device;
};

} // End Namespace

#endif // PERSONREID_H
