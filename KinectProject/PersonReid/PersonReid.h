#ifndef PERSONREID_H
#define PERSONREID_H

#include <QObject>
#include "Feature.h"
#include "types/ColorFrame.h"
#include "types/MaskFrame.h"
#include "types/SkeletonFrame.h"
#include "ml/KMeans.h"
#include "openni/OpenNIDevice.h"

namespace dai {

class PersonReid : public QObject
{
    Q_OBJECT

public:

    void test1();
    void test2();
    void test3();
    void parseDataset();

    void test_DAI4REID();
    QList<shared_ptr<Feature>> train_DAI4REID(QList<int> actors);
    QVector<float> validate_DAI4REID(const QList<int> &actors, const QList<shared_ptr<Feature>>& gallery, int *num_tests = nullptr);

    void test_DAI4REID_Parsed();
    QList<shared_ptr<Feature>> train_DAI4REID_Parsed(QList<int> actors);
    QList<shared_ptr<Feature>> create_gallery_DAI4REID_Parsed();
    QVector<float> validate_DAI4REID_Parsed(const QList<int> &actors, const QList<shared_ptr<Feature>>& gallery, int *num_tests = nullptr);

    void test_CAVIAR4REID();
    QList<shared_ptr<Feature>> train_CAVIAR4REID(QList<int> actors);
    QVector<float> validate_CAVIAR4REID(const QList<int> &actors, const QList<shared_ptr<Feature>>& gallery, int *num_tests = nullptr);

    shared_ptr<Feature> feature_2parts_hist(shared_ptr<ColorFrame> colorFrame, const InstanceInfo& instance_info) const;

    shared_ptr<Feature> feature_joints_hist(ColorFrame &colorFrame, DepthFrame &depthFrame, MaskFrame &maskFrame,
                                            Skeleton &skeleton, const InstanceInfo& instance_info);

    void makeUpJoints(Skeleton& skeleton) const;
    void drawPoint(ColorFrame &colorFrame, int x, int y, RGBColor color = {255, 0, 0}) const;
    SkeletonJoint getCloserJoint(const Point3f& cloudPoint, const QList<SkeletonJoint>& joints) const;
    shared_ptr<MaskFrame> getVoronoiCells(const DepthFrame& depthFrame, const MaskFrame& maskFrame, const Skeleton& skeleton) const;
    shared_ptr<MaskFrame> getVoronoiCellsParallel(const DepthFrame& depthFrame, const MaskFrame& maskFrame, const Skeleton& skeleton) const;
    void colorImageWithVoronoid(ColorFrame &colorFrame, MaskFrame &voronoi) const;
    void highLightMask(ColorFrame &colorFrame, MaskFrame &maskFrame) const;
    void highLightDepth(ColorFrame &colorFrame, DepthFrame &depthFrame) const;
    QHashDataFrames allocateMemory() const;

    QMap<float, int> compute_distances_to_all_samples(const Feature& query, const QList<shared_ptr<Feature>>& gallery);
    int cummulative_match_curve(QMap<float, int> &query_results, QVector<float>& results, int label);
    void normalise_results(QVector<float> &results, int num_accumulated_samples) const;
    void print_results(const QVector<float>& results) const;
    void print_query_results(const QMap<float, int>& query_results, int pos) const;

public slots:
    void execute();

private:
    OpenNIDevice* m_device;
    void printClusters(const QList<Cluster<Feature>>& clusters) const;
    void drawJoints(ColorFrame &colorFrame, const QList<SkeletonJoint>& joints);
    static RGBColor _colors[20];

};

} // End Namespace

#endif // PERSONREID_H
