#include "PersonReid.h"
#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include "openni/OpenNIColorInstance.h"
#include "openni/OpenNIUserTrackerInstance.h"
#include "dataset/CAVIAR4REID/CAVIAR4REID.h"
#include "viewer/InstanceViewerWindow.h"
#include "Config.h"
#include "opencv_utils.h"
#include "types/Histogram.h"
#include <QThread>
#include <climits>

using namespace std;

namespace dai {

RGBColor PersonReid::_colors[20] = {
    255,   0,   0,
      0, 255,   0,
      0,   0, 255,
    255, 255, 255,
    255, 255,   0,
      0, 255, 255,
    255,   0, 255,
      0,   0,   0,
    128,   0,   0,
      0, 128,   0,
      0,   0, 128,
    128, 128,   0,
      0, 128, 128,
    128,   0, 128,
    128, 128, 128,
    255, 128, 128,
    128, 255, 128,
    128, 128, 255,
    100, 255,  10,
    10,  100, 255
};


void PersonReid::execute()
{
    test_OPENNI();
    //7test_CAVIAR4REID();
    QCoreApplication::instance()->quit();
}

QHashDataFrames PersonReid::allocateMemory() const
{
    QHashDataFrames container;
    container.insert(DataFrame::Color, make_shared<ColorFrame>(640, 480));
    container.insert(DataFrame::Depth, make_shared<DepthFrame>(640, 480));
    container.insert(DataFrame::Mask, make_shared<MaskFrame>(640, 480));
    container.insert(DataFrame::Skeleton, make_shared<SkeletonFrame>());
    container.insert(DataFrame::Metadata, make_shared<MetadataFrame>());
    return container;
}

// Test feature of n-parts localised histograms (corresponding to joints) with OpenNI
void PersonReid::test_OPENNI()
{
    // Create Device
    m_device = dai::OpenNIDevice::create("C:/opt/captures/PrimeSense Short-Range (1.09) - 1 user.oni");
    //m_device = dai::OpenNIDevice::create();

    // Create instances
    shared_ptr<dai::OpenNIColorInstance> colorInstance =
            make_shared<dai::OpenNIColorInstance>(m_device);

    shared_ptr<dai::OpenNIUserTrackerInstance> userTrackerInstance =
            make_shared<dai::OpenNIUserTrackerInstance>(m_device);

    // Create memory for colorFrame
    QHashDataFrames readFrames = allocateMemory();

    // Create viewer
    InstanceViewerWindow viewer;
    viewer.show();

    // Open instance
    colorInstance->open();

    // Main Loop
    while (colorInstance->hasNext())
    {
        // Read instances
        colorInstance->readNextFrame(readFrames);
        userTrackerInstance->readNextFrame(readFrames);

        // Get Frames
        auto colorFrame = static_pointer_cast<ColorFrame>(readFrames.value(DataFrame::Color));
        auto depthFrame = static_pointer_cast<DepthFrame>(readFrames.value(DataFrame::Depth));
        auto maskFrame = static_pointer_cast<MaskFrame>(readFrames.value(DataFrame::Mask));
        auto skeletonFrame = static_pointer_cast<SkeletonFrame>(readFrames.value(DataFrame::Skeleton));
        auto metadataFrame = static_pointer_cast<MetadataFrame>(readFrames.value(DataFrame::Metadata));

        // Process
        QList<int> users = skeletonFrame->getAllUsersId();

        // Work with the user inside of the Bounding Box
        if (!users.isEmpty() && !metadataFrame->boundingBoxes().isEmpty())
        {
            int firstUser = users.at(0);
            shared_ptr<Skeleton> skeleton = skeletonFrame->getSkeleton(firstUser);

            // Get ROIs
            BoundingBox bb = metadataFrame->boundingBoxes().first();
            shared_ptr<ColorFrame> roiColor = colorFrame->subFrame(bb);
            shared_ptr<DepthFrame> roiDepth = depthFrame->subFrame(bb);
            shared_ptr<MaskFrame> roiMask = maskFrame->subFrame(bb);

            shared_ptr<Feature> feature = feature_joints_hist(*roiColor, *roiDepth, *roiMask, *skeleton);

            if (feature) {

            }
        }

        // Show
        viewer.showFrame(colorFrame);
        QCoreApplication::processEvents();
    }

    // Close
    colorInstance->close();
    userTrackerInstance->close();
}

shared_ptr<Feature> PersonReid::feature_joints_hist(ColorFrame& colorFrame, DepthFrame& depthFrame,
                                           MaskFrame&  maskFrame, Skeleton& skeleton)
{
    // Build Voronoi cells as a mask
    shared_ptr<MaskFrame> voronoiMask = getVoronoiCells(depthFrame, maskFrame, skeleton);
    colorImageWithVoronoid(colorFrame, *voronoiMask);

    // Compute histograms for each Voronoi cell obtained from joints
    cv::Mat color_mat(colorFrame.height(), colorFrame.width(), CV_8UC3,
                      (void*) colorFrame.getDataPtr(), colorFrame.getStride());

    cv::Mat voronoi_mat(voronoiMask->height(), voronoiMask->width(), CV_8UC1,
                      (void*) voronoiMask->getDataPtr(), voronoiMask->getStride());

    shared_ptr<Feature> feature = make_shared<Feature>();

    foreach (SkeletonJoint joint, skeleton.joints())
    {
        // Convert image to 4096 colors using a color Palette with 16-16-16 levels
        cv::Mat indexed_mat = dai::convertRGB2Indexed161616(color_mat);

        // Calculate histogram and store it
        uchar mask_filter = joint.getType() + 1;
        auto hist = Histogram1s::create(indexed_mat, {0, 4095}, voronoi_mat, mask_filter);
        feature->addHistogram(*hist);
    }

    return feature;
}

// Extract Voronoi cells as a mask
shared_ptr<MaskFrame> PersonReid::getVoronoiCells(const DepthFrame& depthFrame, const MaskFrame& maskFrame, const Skeleton& skeleton) const
{
    Q_ASSERT(depthFrame.width() == maskFrame.width() && depthFrame.height() == maskFrame.height());

    shared_ptr<MaskFrame> result = static_pointer_cast<MaskFrame>(maskFrame.clone());
    QList<SkeletonJoint> joints = skeleton.joints();

    for (int i=0; i<depthFrame.height(); ++i)
    {
        uint16_t* depth = depthFrame.getRowPtr(i);
        uint8_t* mask = result->getRowPtr(i);

        for (int j=0; j<depthFrame.width(); ++j)
        {
            if (mask[j] > 0)
            {
                Point3f point(0, 0, depth[j]);

                m_device->convertDepthCoordinatesToJoint(j+depthFrame.offset()[0], i+depthFrame.offset()[1],
                        depth[j], &point[0], &point[1]);

                SkeletonJoint closerJoint = getCloserJoint(point, joints);
                mask[j] = closerJoint.getType()+1;
            }
        }
    }

    return result;
}

void PersonReid::colorImageWithVoronoid(ColorFrame& colorFrame, MaskFrame& voronoi) const
{
    Q_ASSERT(colorFrame.width() == voronoi.width() && colorFrame.height() == voronoi.height());

    for (int i=0; i<voronoi.height(); ++i)
    {
        RGBColor* pixel = colorFrame.getRowPtr(i);
        uint8_t* mask = voronoi.getRowPtr(i);

        for (int j=0; j<voronoi.width(); ++j) {
            if (mask[j] > 0) {
                pixel[j] = _colors[mask[j]-1];
            }
        }
    }
}

void PersonReid::highLightMask(ColorFrame &colorFrame, MaskFrame &maskFrame) const
{
    Q_ASSERT(colorFrame.width() == maskFrame.width() && colorFrame.height() == maskFrame.height());

    // highlight Mask
    for (int i=0; i<maskFrame.height(); ++i) {
        uint8_t* mask = maskFrame.getRowPtr(i);
        RGBColor* color = colorFrame.getRowPtr(i);
        for (int j=0; j<maskFrame.width(); ++j){
            if (mask[j] > 0) {
                color[j].green = 255;
            }
        }
    }
}

/*void drawJoints() {
    // Draw Joints
    foreach (SkeletonJoint joint, joints) {
        float x, y;
        m_device->convertJointCoordinatesToDepth(joint.getPosition()[0],
                joint.getPosition()[1], joint.getPosition()[2], &x, &y);
        drawPoint(colorFrame, x, y);
    }
}*/

SkeletonJoint PersonReid::getCloserJoint(const Point3f& cloudPoint, const QList<SkeletonJoint>& joints) const
{
    SkeletonJoint minJoint;
    float minDistance = 999999999.0f; //numeric_limits<float>::max();

    foreach (SkeletonJoint joint, joints)
    {
        float distance = Point3f::euclideanDistanceSquared(cloudPoint, joint.getPosition());

        if (distance < minDistance) {
            minDistance = distance;
            minJoint = joint; // copy
        }
    }

    return minJoint;
}

void PersonReid::drawPoint(shared_ptr<ColorFrame> colorFrame, int x, int y) const
{
    if (x >= 1 && (x+1) < 640 && y >= 1 && (y+1) < 480)
    {
        shared_ptr<ColorFrame> subColor = colorFrame->subFrame(y-1, x-1, 3, 3);

        for (int i=0; i<subColor->height(); ++i)
        {
            RGBColor* color = subColor->getRowPtr(i);

            for (int j=0; j<subColor->width(); ++j)
            {
                color[j].red = 255;
                color[j].green = 0;
                color[j].blue = 0;
            }
        }
    }
}

// Test feature of 2-parts Histogram (upper and lower) with CAVIAR4REID
void PersonReid::test_CAVIAR4REID()
{
    QList<int> actors = {
         3,  4,  6,  7,  8,  9, 12, 15, 16, 18,
        19, 20, 21, 22, 23, 24, 27, 28, 29, 30,
        31, 32, 33, 34, 35, 36, 37, 38, 40, 41,
        44, 45, 46, 47, 50, 52, 55, 57, 58, 59,
        60, 62, 64, 65, 66, 67, 68, 69, 70, 71
    };

    // Training
    QList<shared_ptr<Feature>> gallery = train_CAVIAR4REID(actors);

    // Validation
    int num_tests = 0;
    QVector<float> results = validate_CAVIAR4REID(actors, gallery, &num_tests);

    // Show Results
    normalise_results(results, num_tests);
    print_results(results);
}

QList<shared_ptr<Feature>> PersonReid::train_CAVIAR4REID(QList<int> actors)
{
    Dataset* dataset = new CAVIAR4REID;
    dataset->setPath("C:/datasets/CAVIAR4REID");
    const DatasetMetadata& metadata = dataset->getMetadata();

    InstanceViewerWindow viewer;
    viewer.show();

    QList<shared_ptr<Feature>> samples;
    QList<shared_ptr<Feature>> actor_samples;
    QList<shared_ptr<Feature>> centroids;

    // For each actor, compute the feature that minimises the distance to each other sample of
    // the same actor.
    foreach (int actor, actors)
    {
        QList<shared_ptr<InstanceInfo>> instances = metadata.instances(DataFrame::Color,
                                                                       {actor},
                                                                       {1},
                                                                       DatasetMetadata::ANY_LABEL);
        actor_samples.clear();

        foreach (shared_ptr<InstanceInfo> instance_info, instances)
        {
            /*std::string fileName = instance_info->getFileName(DataFrame::Color).toStdString();

            printf("actor %i sample %i file %s\n", instance_info->getActor(),
                   instance_info->getSample(),
                   fileName.c_str());*/

            // Get Sample
            shared_ptr<StreamInstance> instance = dataset->getInstance(*instance_info, DataFrame::Color);
            instance->open();
            QHashDataFrames readFrames;
            instance->readNextFrame(readFrames);
            shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(readFrames.values().at(0));

            // Feature Extraction
            shared_ptr<Feature> feature = feature_2parts_hist(colorFrame, *instance_info);
            samples << feature;
            actor_samples << feature;

            // Output
            viewer.showFrame(colorFrame);

            // Process Events and sleep
            std::fflush(stdout);
            QCoreApplication::processEvents();
            QThread::msleep(800);
        }

        shared_ptr<Feature> selectedFeature = Feature::minFeature(actor_samples);
        centroids << selectedFeature;
        printf("Init. Centroid %i = actor %i %i\n", actor, selectedFeature->label().getActor(),
                                                       selectedFeature->label().getSample());
    }

    // Learn A: Learning a signature is the same as clustering the input data into num_actor sets
    // and use the centroid of each cluster as model.
    auto kmeans = KMeans<Feature>::execute(samples, actors.size(), centroids);
    printClusters(kmeans->getClusters());

    QList<shared_ptr<Feature>> gallery;

    foreach (auto cluster, kmeans->getClusters()) {
        gallery << cluster.centroid;
    }

    // Learn B: Use the signature that minimises the distance to the rest of signatures
    // of each actor.
    //QList<shared_ptr<Feature>> gallery = centroids;

    return gallery;
}

QVector<float> PersonReid::validate_CAVIAR4REID(const QList<int>& actors, const QList<shared_ptr<Feature> > &gallery, int *num_tests)
{
    Dataset* dataset = new CAVIAR4REID;
    dataset->setPath("C:/datasets/CAVIAR4REID");
    const DatasetMetadata& metadata = dataset->getMetadata();

    QList<shared_ptr<InstanceInfo>> instances = metadata.instances(DataFrame::Color,
                                                                   actors,
                                                                   {2},
                                                                   DatasetMetadata::ANY_LABEL);

    if (num_tests) {
        *num_tests = instances.size();
    }

    QVector<float> results(actors.size());

    foreach (shared_ptr<InstanceInfo> instance_info, instances)
    {
        // Get Sample
        shared_ptr<StreamInstance> instance = dataset->getInstance(*instance_info, DataFrame::Color);
        instance->open();
        QHashDataFrames readFrames;
        instance->readNextFrame(readFrames);
        shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(readFrames.values().at(0));

        // Feature Extraction
        shared_ptr<Feature> query = feature_2parts_hist(colorFrame, *instance_info);

        // CMC
        QMap<float, int> query_results = compute_distances_to_all_samples(*query, gallery);
        int pos = cummulative_match_curve(query_results, results, query->label().getActor());
        cout << "Results for actor " << instance_info->getActor() << " " << instance_info->getSample() << endl;
        print_query_results(query_results, pos);
        cout << "--------------------------" << endl;
    }

    return results;
}

QMap<float, int> PersonReid::compute_distances_to_all_samples(const Feature& query, const QList<shared_ptr<Feature> > &gallery)
{
    QMap<float, int> query_results;

    foreach (shared_ptr<Feature> target, gallery) {
        float distance = query.distance(*target);
        query_results.insert(distance, target->label().getActor());
    }

    return query_results;
}

int PersonReid::cummulative_match_curve(QMap<float, int>& query_results, QVector<float>& results, int label)
{
    // Count results as a CMC
    auto it = query_results.constBegin();
    int pos = -1;
    int i = 0;

    // Find position of the correct match
    while (it != query_results.constEnd() && pos == -1) {
        if (it.value() == label) {
            pos = i;
        }
        ++it;
        ++i;
    }

    // Accumulate for ranks
    for (int i=pos; i<results.size(); ++i) {
        results[i]++;
    }

    return pos;
}

void PersonReid::normalise_results(QVector<float>& results, int num_accumulated_samples) const
{
    for (int i=0; i<results.size(); ++i) {
        results[i] = results[i] / float(num_accumulated_samples);
    }
}

void PersonReid::print_query_results(const QMap<float, int>& query_results, int pos) const
{
    int i = 0;

    for (auto it = query_results.constBegin(); it != query_results.constEnd(); ++it) {
        if (i == pos) cout << "*";
        cout << "dist " << it.key() << " actor " << it.value() << endl;
        ++i;
    }
}

void PersonReid::print_results(const QVector<float> &results) const
{
    cout << "Rank" << "\t" << "Matching Rate" << endl;
    for (int i=0; i<results.size(); ++i) {
        cout << (i+1) << "\t" << results[i] << endl;
    }
}

shared_ptr<Feature> PersonReid::feature_2parts_hist(shared_ptr<ColorFrame> colorFrame, const InstanceInfo& instance_info) const
{
    cv::Mat inputImg(colorFrame->height(), colorFrame->width(),
                 CV_8UC3, (void*)colorFrame->getDataPtr(), colorFrame->getStride());

    cv::Mat upper_mask, lower_mask;
    dai::computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask);

    // Color Palette with 16-16-16 levels
    cv::Mat indexedImg = dai::convertRGB2Indexed161616(inputImg);

    // Compute the histogram for the upper (torso) and lower (leggs) parts of each frame in the buffer
    auto u_hist = Histogram1s::create(indexedImg, {0, 4095}, upper_mask);
    auto l_hist = Histogram1s::create(indexedImg, {0, 4095}, lower_mask);

    dai::colorImageWithMask(inputImg, inputImg, upper_mask, lower_mask);

    // Create feature
    shared_ptr<Feature> feature = make_shared<Feature>(instance_info);
    feature->addHistogram(*u_hist);
    feature->addHistogram(*l_hist);

    return feature;
}

void PersonReid::printClusters(const QList<Cluster<Feature> > &clusters) const
{
    int i = 0;

    foreach (Cluster<Feature> cluster, clusters)
    {
        if (!cluster.samples.isEmpty())
        {
            shared_ptr<Feature> centroid = cluster.centroid;
            printf("Cluster %i (size=%i, actor=%i %i)\n", i+1, cluster.samples.size(), centroid->label().getActor(), centroid->label().getSample());

            foreach (shared_ptr<Feature> items, cluster.samples) {
                const InstanceInfo& label = items->label();
                printf("(%i %i) ", label.getActor(), label.getSample());
            }

            cout << endl << endl;
            ++i;
        }
    }
}

} // End Namespace
