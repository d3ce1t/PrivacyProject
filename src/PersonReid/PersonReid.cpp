#include "PersonReid.h"
#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include "openni/OpenNIColorInstance.h"
#include "openni/OpenNIUserTrackerInstance.h"
#include "dataset/CAVIAR4REID/CAVIAR4REID.h"
#include "dataset/DAI4REID/DAI4REID.h"
#include "dataset/DAI4REID_Parsed/DAI4REID_Parsed.h"
#include "dataset/IASLAB_RGBD_ID/IASLAB_RGBD_ID.h"
#include "Config.h"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv_utils.h"
#include "types/Histogram.h"
#include <QThread>
#include <future>
#include "JointHistograms.h"
#include "DistancesFeature.h"
#include "RegionDescriptor.h"
#include "DescriptorSet.h"
#include <QtConcurrent>
#include <cstdlib>

namespace dai {

RGBColor PersonReid::_colors[20] = {
    {255,   0,   0},
    {  0, 255,   0},
    {  0,   0, 255},
    {255, 255, 255},
    {255, 255,   0},
    {  0, 255, 255},
    {255,   0, 255},
    {  0,   0,   0},
    {128,   0,   0},
    {  0, 128,   0},
    {  0,   0, 128},
    {128, 128,   0},
    {  0, 128, 128},
    {128,   0, 128},
    {128, 128, 128},
    {255, 128, 128},
    {128, 255, 128},
    {128, 128, 255},
    {100, 255,  10},
    {10,  100, 255}
};

void PersonReid::execute()
{
    // Select dataset
    Dataset* dataset = new IASLAB_RGBD_ID;
    dataset->setPath("/Volumes/Files/Datasets/IASLAB-RGBD-ID");
    //dataset->setPath("/Volumes/MacHDD/Users/jpadilla/Datasets/IASLAB-RGBD-ID");
    //dataset->setPath("/files/IASLAB-RGBD-ID");
    //Dataset* dataset = new DAI4REID_Parsed;
    //dataset->setPath("/Volumes/MacHDD/Users/jpadilla/Datasets/DAI4REID_Parsed");
    //dataset->setPath("/files/DAI4REID_Parsed");

    // Select actors
    QList<int> actors = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}; // IAS-Lab RGBD-ID
    //QList<int> actors = {3, 4, 5, 7, 8, 9, 10, 11};
    //QList<int> actors = {1, 2, 3, 4, 5}; //DAI4REID
    /*QList<int> actors = { // CAVIAR4REID
         3,  4,  6,  7,  8,  9, 12, 15, 16, 18,
        19, 20, 21, 22, 23, 24, 27, 28, 29, 30,
        31, 32, 33, 34, 35, 36, 37, 38, 40, 41,
        44, 45, 46, 47, 50, 52, 55, 57, 58, 59,
        60, 62, 64, 65, 66, 67, 68, 69, 70, 71
    };*/

    // Start
    QVector<float> results(actors.size());
    int num_tests = 0;

    // Train cam1, Test cam2 and viceversa
    for (int i=0; i<2; ++i)
    {
        // Training (camera 1)
        QMultiMap<int, DescriptorPtr> gallery = train(dataset, actors, i==0 ? 1 : 2);

        /*for (DescriptorPtr target : gallery) {
            qDebug() << target->label().getActor() << target->label().getSample();
        }*/

        // Validation (camera 2)
        validate(dataset, actors, i==0 ? 2 : 1, gallery, results, &num_tests);
    }

    // Show Results
    normalise_results(results, num_tests);
    print_results(results);

    // Quit
    QCoreApplication::instance()->quit();
}

DescriptorPtr PersonReid::computeFeature(Dataset* dataset, shared_ptr<InstanceInfo> instance_info)
{
    // Get Sample
    shared_ptr<StreamInstance> instance = dataset->getInstance(*instance_info, DataFrame::Color);

    // Open Instances
    instance->open();

    // Read frames
    QHashDataFrames readFrames;
    instance->readNextFrame(readFrames);

    // Get Frames
    auto colorFrame = static_pointer_cast<ColorFrame>(readFrames.value(DataFrame::Color));
    auto depthFrame = static_pointer_cast<DepthFrame>(readFrames.value(DataFrame::Depth));
    auto maskFrame = static_pointer_cast<MaskFrame>(readFrames.value(DataFrame::Mask));
    auto skeletonFrame = static_pointer_cast<SkeletonFrame>(readFrames.value(DataFrame::Skeleton));
    shared_ptr<Skeleton> skeleton = skeletonFrame->getSkeleton(skeletonFrame->getAllUsersId().at(0));

    // Process
    //DescriptorPtr feature = feature_global_hist(*colorFrame, *maskFrame, *instance_info);
    DescriptorPtr feature = feature_joints_hist(*colorFrame, *depthFrame, *maskFrame, *skeleton, *instance_info);
    //DescriptorPtr feature = feature_region_descriptor(*colorFrame, *depthFrame, *maskFrame, *skeleton, *instance_info);
    //DescriptorPtr feature = feature_pointinterest_descriptor(*colorFrame, *maskFrame, *instance_info);
    //DescriptorPtr feature = feature_skeleton_distances(*colorFrame, *skeleton, *instance_info);
    //DescriptorPtr feature = feature_joint_descriptor(*colorFrame, *depthFrame, *maskFrame, *skeleton, *instance_info);
    //DescriptorPtr feature = feature_fusion(*colorFrame, *depthFrame, *maskFrame, *skeleton, *instance_info);

    // Close Instances
    instance->close();

    return feature;
}

/**
 * Realmente este método no realiza un entrenamiento, únicamente carga como galería todas las imágenes
 * del banco de datos. Sería equivalente a aprender todos.
 *
 * @brief PersonReid::train_multiple
 * @param dataset
 * @param actors
 * @param camera
 * @return
 */
QMultiMap<int, DescriptorPtr> PersonReid::train(Dataset* dataset, QList<int> actors, int camera)
{
    const DatasetMetadata& metadata = dataset->getMetadata();
    QMultiMap<int, DescriptorPtr> gallery;

    for (int actor : actors)
    {
        int samples_processed = 0; // per actor
        QElapsedTimer timer;
        timer.start();

        QList<shared_ptr<InstanceInfo>> instances = metadata.instances({actor}, {camera}, DatasetMetadata::ANY_LABEL);

        // Single thread
        /*int i = 0;

        for (shared_ptr<InstanceInfo> instance_info : instances) {
            if (i % 2 == 0) {
                DescriptorPtr feature = PersonReid::computeFeature(dataset, instance_info);

                if (feature) {
                    gallery.insert(feature->label().getActor(), feature);
                    samples_processed++;
                }

                // Show
                //show_images(colorFrame, maskFrame, depthFrame, skeleton);
                qDebug("actor %i sample %i fps %f", feature->label().getActor(), feature->label().getSample(), float(samples_processed )/ timer.elapsed() * 1000.0f);
            }
            i++;
        }*/

        // Parallel version
        std::vector<QFuture<DescriptorPtr>> workers;

        //int i = 0;
        for (shared_ptr<InstanceInfo> instance_info : instances) {
            //if (i % 2 == 0)
                workers.push_back( QtConcurrent::run(this, &PersonReid::computeFeature, dataset, instance_info) );
            //i++;
        }

        for (QFuture<DescriptorPtr>& f : workers) {

            DescriptorPtr feature = f.result();

            if (feature) {
                gallery.insert(feature->label().getActor(), feature);
                samples_processed++;
            }

            // Show
            //show_images(colorFrame, maskFrame, depthFrame, skeleton);
            qDebug("actor %i sample %i fps %f", feature->label().getActor(), feature->label().getSample(), float(samples_processed )/ timer.elapsed() * 1000.0f);
        }

        qDebug() << "Gallery size" << gallery.size();
    }

    return gallery;
}

void PersonReid::validate(Dataset* dataset, const QList<int> &actors, int camera, const QMultiMap<int, DescriptorPtr>& gallery, QVector<float>& results, int *num_tests)
{
    const DatasetMetadata& metadata = dataset->getMetadata();
    QList<shared_ptr<InstanceInfo>> instances = metadata.instances(actors, {camera}, DatasetMetadata::ANY_LABEL);
    std::vector<QFuture<DescriptorPtr>> workers;
    int total_tests = 0;

    // Start validation
    for (shared_ptr<InstanceInfo> instance_info : instances) {
        workers.push_back( QtConcurrent::run(this, &PersonReid::computeFeature, dataset, instance_info) );
    }

    for (QFuture<DescriptorPtr>& f : workers) {

        DescriptorPtr query = f.result();

        if (query) {
            // CMC: Build ranking
            QMap<float, int> query_results; // distance, actor

            for (int target : actors) {
                // Get all of the samples of the target actor
                QList<DescriptorPtr> samples = gallery.values(target);
                float distance = Descriptor::minDistanceParallel(query, samples);
                query_results.insertMulti(distance, target);
            }

            int pos = cummulative_match_curve(query_results, results, query->label().getActor());
            std::string fileName = query->label().getFileName(DataFrame::Color).toStdString();
            qDebug("Results for actor %i sample %i file %s (pos=%i)", query->label().getActor(), query->label().getSample(), fileName.c_str(), pos+1);
            print_query_results(query_results, pos);
            qDebug() << "--------------------------";
            total_tests++;
        }
    }

    if (num_tests) {
        *num_tests += total_tests;
    }
}

void PersonReid::show_images(shared_ptr<ColorFrame> colorFrame, shared_ptr<MaskFrame> maskFrame, shared_ptr<DepthFrame> depthFrame, shared_ptr<Skeleton> skeleton)
{
    Q_UNUSED(depthFrame);

    // Mask Frame
    /*cv::Mat mask_mat(maskFrame->height(), maskFrame->width(), CV_8UC1,
                     (void*) maskFrame->getDataPtr(), maskFrame->getStride());

    filterMask(mask_mat, mask_mat, [&](uchar in, uchar &out){
        out = in == 1 ? 255 : 0;
    });*/

    // Color Frame
    cv::Mat color_src(colorFrame->height(), colorFrame->width(), CV_8UC3,
                      (void*) colorFrame->getDataPtr(), colorFrame->getStride());

    cv::cvtColor(color_src, color_src, CV_BGR2RGB);

    /*for_each_pixel<cv::Vec3b>(color_src, [&](cv::Vec3b& pixel, int row, int column) {
        if (mask_mat.at<uchar>(row, column) == 0) {
            pixel[0] = 0;
            pixel[1] = 0;
            pixel[2] = 0;
        }
    });*/

    // Depth Frame
    /*cv::Mat depth_mat(depthFrame->height(), depthFrame->width(), CV_16UC1,
                     (void*) depthFrame->getDataPtr(), depthFrame->getStride());
    */

    // Skeleton Frame
    //ColorFramePtr skeleton_color = make_shared<ColorFrame>(colorFrame->width(), colorFrame->height());
    //skeleton_color->setOffset(colorFrame->offset());
    drawSkeleton(*(colorFrame.get()), *skeleton);
    /*cv::Mat skeleton_mat(skeleton_color->height(), skeleton_color->width(), CV_8UC3,
                         (void*) skeleton_color->getDataPtr(), skeleton_color->getStride());*/

    // Show frames
    cv::imshow("Original", color_src);
    //cv::imshow("Mask", mask_mat);
    //cv::imshow("Depth", depth_mat);
    //cv::imshow("Skeleton", skeleton_mat);
    cv::waitKey(1);

    /*if (instance_info->getSample() == 440) {
        cv::imwrite("paco_src.png", color_src);
        cv::imwrite("paco_mask.png", mask_mat);
        cv::imwrite("paco_skeleton.png", skeleton_mat);
        cv::imwrite("paco_voronoi.png", color_mat);
        cv::waitKey(3000);
    }*/
}

QMap<float, int> PersonReid::compute_distances_to_all_samples(const Descriptor& query, const QList<DescriptorPtr > &gallery)
{
    QMap<float, int> query_results; // distance, actor

    for (DescriptorPtr target : gallery) {
        float distance = query.distance(*target);
        query_results.insertMulti(distance, target->label().getActor());
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
        qDebug("%c dist %f actor %i", i == pos?'*':' ', it.key(), it.value());
        ++i;
    }
}

void PersonReid::print_results(const QVector<float> &results) const
{
    qDebug() << "Rank" << "\t" << "Matching Rate";
    for (int i=0; i<results.size(); ++i) {
        qDebug() << (i+1) << "\t" << results[i];
    }
}

DescriptorPtr PersonReid::feature_fusion(ColorFrame &colorFrame, DepthFrame &depthFrame, MaskFrame &maskFrame,
                                        Skeleton &skeleton, const InstanceInfo& instance_info) const
{
    shared_ptr<DescriptorSet> feature = make_shared<DescriptorSet>(instance_info, colorFrame.getIndex());

    //DescriptorPtr joints_hist = feature_joints_hist(colorFrame, depthFrame, maskFrame, skeleton, instance_info);
    DescriptorPtr skeleton_distances = feature_skeleton_distances(colorFrame, skeleton, instance_info);
    DescriptorPtr joint_desc = feature_joint_descriptor(colorFrame, depthFrame, maskFrame, skeleton, instance_info);
    DescriptorPtr region_desc = feature_region_descriptor(colorFrame, depthFrame, maskFrame, skeleton, instance_info);

    //feature->addDescriptor(joints_hist);
    feature->addDescriptor(skeleton_distances);
    feature->addDescriptor(joint_desc);
    feature->addDescriptor(region_desc);

    return feature;
}

DescriptorPtr PersonReid::feature_global_hist(ColorFrame& colorFrame, MaskFrame&  maskFrame, const InstanceInfo &instance_info) const
{
    // Compute histograms global histogram of the silhouette
    cv::Mat color_mat(colorFrame.height(), colorFrame.width(), CV_8UC3,
                      (void*) colorFrame.getDataPtr(), colorFrame.getStride());

    cv::Mat mask_mat(maskFrame.height(), maskFrame.width(), CV_8UC1,
                      (void*) maskFrame.getDataPtr(), maskFrame.getStride());

    // Convert image to HSV
    cv::Mat hsv_mat;
    cv::cvtColor(color_mat, hsv_mat, CV_RGB2HSV);
    std::vector<cv::Mat> hsv_planes;
    cv::split(hsv_mat, hsv_planes);
    cv::Mat indexed_mat = hsv_planes[0];

    shared_ptr<JointHistograms1c> feature = make_shared<JointHistograms1c>(instance_info, colorFrame.getIndex());
    auto hist = Histogram1c::create(indexed_mat, {0, 255}, mask_mat);
    feature->addHistogram(*hist);

    //colorImageWithVoronoid(colorFrame, maskFrame);

    return feature;
}

/**
 * Característica que divide la silueta de la persona en 15 voronoi cells que tienen como centro
 * las articulaciones del esqueleto. La región de cada célula se describe mediante el histograma
 * de color HSV (Full). Concretamente, se computa el histograma utilizando el canal H.
 *
 * La misma característica se puede utilizar añadiendo más articulaciones
 *
 * @brief PersonReid::feature_joints_hist
 * @param colorFrame
 * @param depthFrame
 * @param maskFrame
 * @param skeleton
 * @param instance_info
 * @return
 */
DescriptorPtr PersonReid::feature_joints_hist(ColorFrame& colorFrame, DepthFrame& depthFrame,
                                           MaskFrame&  maskFrame, Skeleton& skeleton, const InstanceInfo &instance_info) const
{
    QSet<SkeletonJoint::JointType> ignore_joints = {
        //SkeletonJoint::JOINT_HEAD,
        //SkeletonJoint::JOINT_LEFT_HAND,
        //SkeletonJoint::JOINT_RIGHT_HAND,
        //SkeletonJoint::JOINT_LEFT_FOOT,
        //SkeletonJoint::JOINT_RIGHT_FOOT
    };

    // Build Voronoi cells as a mask
    Skeleton skeleton_tmp = skeleton; // copy
    //makeUpJoints(skeleton, true);
    //makeUpOnlySomeJoints(skeleton); // Modify passed skeleton in order to view changes in show_images method (called outside of this method)
    shared_ptr<MaskFrame> voronoiMask = getVoronoiCells(depthFrame, maskFrame, skeleton);

    // Compute histograms for each Voronoi cell obtained from joints
    cv::Mat color_mat(colorFrame.height(), colorFrame.width(), CV_8UC3,
                      (void*) colorFrame.getDataPtr(), colorFrame.getStride());

    cv::Mat voronoi_mat(voronoiMask->height(), voronoiMask->width(), CV_8UC1,
                      (void*) voronoiMask->getDataPtr(), voronoiMask->getStride());

    // Convert image to HSV
    cv::Mat hsv_mat;
    cv::cvtColor(color_mat, hsv_mat, CV_RGB2HSV);
    std::vector<cv::Mat> hsv_planes;
    cv::split(hsv_mat, hsv_planes);
    cv::Mat indexed_mat = hsv_planes[0];

    shared_ptr<JointHistograms1c> feature = make_shared<JointHistograms1c>(instance_info, colorFrame.getIndex());

    for (const SkeletonJoint& joint : skeleton_tmp.joints()) // I use the skeleton of 15 or 20 joints not the modified one
    {
        if (!ignore_joints.contains(joint.getType())) {
            uchar mask_filter = joint.getType() + 1;
            auto hist = Histogram1c::create(indexed_mat, {0, 180}, voronoi_mat, mask_filter);
            feature->addHistogram(*hist);
        }
    }

    //colorImageWithVoronoid(colorFrame, *voronoiMask);

    return feature;
}

/**
 * Característica que divide la silueta de la persona en 15 voronoi cells que tienen como centro
 * las articulaciones del esqueleto. De la región de cada célula se extraen puntos característicos,
 * como SURF, SIFT o FAST, que son luego empleados para describir la región empleando un descriptor
 * SURF o SIFT.
 *
 * @brief PersonReid::feature_region_descriptor
 * @param colorFrame
 * @param depthFrame
 * @param maskFrame
 * @param skeleton
 * @param instance_info
 * @return
 */
DescriptorPtr PersonReid::feature_region_descriptor(ColorFrame &colorFrame, DepthFrame &depthFrame, MaskFrame &maskFrame,
                                        Skeleton &skeleton, const InstanceInfo& instance_info) const
{
    cv::Mat color_mat(colorFrame.height(), colorFrame.width(), CV_8UC3,
                      (void*) colorFrame.getDataPtr(), colorFrame.getStride());

    // Build Voronoi cells as a mask
    Skeleton skeleton_tmp = skeleton; // copy
    //makeUpJoints(skeleton_tmp);
    shared_ptr<MaskFrame> voronoiMask = getVoronoiCells(depthFrame, maskFrame, skeleton_tmp);

    cv::Mat voronoi_mat(voronoiMask->height(), voronoiMask->width(), CV_8UC1,
                      (void*) voronoiMask->getDataPtr(), voronoiMask->getStride());

    // Convert image to gray for point detector
    cv::Mat gray_mat;
    cv::cvtColor(color_mat, gray_mat, CV_RGB2GRAY);

    auto computeFeature = [&](const SkeletonJoint& joint) -> cv::Mat
    {
        cv::FastFeatureDetector detector(20); // fast thresold = 20
        cv::SiftDescriptorExtractor extractor;

        uchar mask_filter = joint.getType() + 1;
        cv::Mat joint_mask;

        // Create a mask for a given joint with 1 values in pixels matching this
        // joint in the voronoit mat
        filterMask(voronoi_mat, joint_mask, [&](uchar in, uchar& out) {
            out = in == mask_filter ? 1 : 0;
        });

        // Detect Keypoints
        std::vector<cv::KeyPoint> keypoints;
        detector.detect( gray_mat, keypoints, joint_mask);

        // Calculate descriptor
        cv::Mat descriptor;
        extractor.compute(gray_mat, keypoints, descriptor);

        /*cv::Mat img_keypoints;
        cv::drawKeypoints(gray_mat, keypoints, img_keypoints, cv::Scalar::all(-1), cv::DrawMatchesFlags::DEFAULT );
        cv::imshow("points", img_keypoints);
        cv::waitKey(1);*/

        // Return
        return descriptor;
    };

    /*QSet<SkeletonJoint::JointType> ignore_joints = {SkeletonJoint::JOINT_HEAD,
                                                    SkeletonJoint::JOINT_LEFT_HAND,
                                                    SkeletonJoint::JOINT_RIGHT_HAND};*/
    std::vector<QFuture<cv::Mat>> workers;

    for (const SkeletonJoint& joint : skeleton.joints()) // I use the skeleton of 15 or 20 joints not the temp one.
    {
        //if (ignore_joints.contains(joint.getType()))
        //    continue;

        workers.push_back( QtConcurrent::run(computeFeature, joint) );
    }

    shared_ptr<RegionDescriptor> feature = make_shared<RegionDescriptor>(instance_info, colorFrame.getIndex());

    for (QFuture<cv::Mat>& f : workers) {
        feature->addDescriptor(f.result());
    }

    //colorImageWithVoronoid(colorFrame, *voronoiMask);

    return feature;
}

DescriptorPtr PersonReid::feature_pointinterest_descriptor(ColorFrame &colorFrame, MaskFrame &maskFrame,
                                                        const InstanceInfo& instance_info) const
{
    cv::Mat color_mat(colorFrame.height(), colorFrame.width(), CV_8UC3,
                      (void*) colorFrame.getDataPtr(), colorFrame.getStride());

    cv::Mat mask_mat(maskFrame.height(), maskFrame.width(), CV_8UC1,
                      (void*) maskFrame.getDataPtr(), maskFrame.getStride());

    // Convert image to gray for point detector
    cv::Mat gray_mat;
    cv::cvtColor(color_mat, gray_mat, CV_RGB2GRAY);

    shared_ptr<RegionDescriptor> feature = make_shared<RegionDescriptor>(instance_info, colorFrame.getIndex());
    cv::SurfFeatureDetector detector; // fast thresold = 20
    cv::SurfDescriptorExtractor extractor;

    // Detect Keypoints
    std::vector<cv::KeyPoint> keypoints;
    detector.detect( gray_mat, keypoints, mask_mat);

    // Calculate descriptor
    cv::Mat descriptor;
    extractor.compute(gray_mat, keypoints, descriptor);

    /*cv::Mat img_keypoints;
    cv::drawKeypoints(gray_mat, keypoints, img_keypoints, cv::Scalar::all(-1), cv::DrawMatchesFlags::DEFAULT );
    cv::imshow("points", img_keypoints);
    cv::waitKey(1);*/

    feature->addDescriptor(descriptor);

    return feature;
}

/**
 * Característica que utiliza cada articulación del esqueleto como punto característico. Estos puntos son utilizados
 * para describir cada articulación mediante un descriptor SURF o SIFT.
 *
 * @brief PersonReid::feature_joint_descriptor
 * @param colorFrame
 * @param skeleton
 * @param instance_info
 * @return
 */
DescriptorPtr PersonReid::feature_joint_descriptor(ColorFrame &colorFrame, DepthFrame &depthFrame, MaskFrame &maskFrame,
                                                   Skeleton &skeleton, const InstanceInfo& instance_info) const
{
    cv::Mat color_mat(colorFrame.height(), colorFrame.width(), CV_8UC3,
                      (void*) colorFrame.getDataPtr(), colorFrame.getStride());

    // Convert image to gray for point detector
    cv::Mat gray_mat;
    cv::cvtColor(color_mat, gray_mat, CV_RGB2GRAY);

    QSet<SkeletonJoint::JointType> ignore_joints = {//SkeletonJoint::JOINT_HEAD,
                                                    //SkeletonJoint::JOINT_CENTER_SHOULDER,
                                                    //SkeletonJoint::JOINT_LEFT_SHOULDER,
                                                    //SkeletonJoint::JOINT_RIGHT_SHOULDER,
                                                    //SkeletonJoint::JOINT_LEFT_ELBOW,
                                                    //SkeletonJoint::JOINT_RIGHT_ELBOW,
                                                    //SkeletonJoint::JOINT_LEFT_HAND,
                                                    //SkeletonJoint::JOINT_RIGHT_HAND,
                                                    //SkeletonJoint::JOINT_SPINE,
                                                    //SkeletonJoint::JOINT_LEFT_HIP,
                                                    //SkeletonJoint::JOINT_RIGHT_HIP,
                                                    //SkeletonJoint::JOINT_LEFT_KNEE,
                                                    //SkeletonJoint::JOINT_RIGHT_KNEE,
                                                    //SkeletonJoint::JOINT_LEFT_FOOT,
                                                    //SkeletonJoint::JOINT_RIGHT_FOOT
                                                   };

    // Make up joints
    Skeleton skeleton_tmp = skeleton; // copy
    //makeUpJoints(skeleton_tmp, true);

    shared_ptr<MaskFrame> voronoiMask = getVoronoiCells(depthFrame, maskFrame, skeleton_tmp);
    vector<cv::KeyPoint> key_points;

    // Get Key Points from the Skeleton Joints
    for (const SkeletonJoint& joint : skeleton_tmp.joints())
    {
        if (ignore_joints.contains(joint.getType()))
            continue;

        key_points.push_back(createKeyPoint(joint, skeleton_tmp, voronoiMask));
    }

    // Calculate descriptor
    cv::SiftDescriptorExtractor extractor;
    cv::Mat descriptor;
    extractor.compute(gray_mat, key_points, descriptor);

    // Add to the feature
    shared_ptr<RegionDescriptor> feature = make_shared<RegionDescriptor>(instance_info, colorFrame.getIndex());
    feature->addDescriptor(descriptor);

    /*cv::Mat img_keypoints;
    cv::drawKeypoints(gray_mat, key_points, img_keypoints, cv::Scalar::all(-1), cv::DrawMatchesFlags::DEFAULT );
    cv::imshow("points", img_keypoints);
    cv::waitKey(200);*/

    return feature;
}

cv::KeyPoint PersonReid::createKeyPoint(const SkeletonJoint& joint, const Skeleton& skeleton, shared_ptr<MaskFrame> voronoi) const
{
    // Convert 3D point to 2D
    const Point3f& point_3d = joint.getPosition();
    cv::Point2f center_point_2d;

    skeleton.convertCoordinatesToDepth(point_3d[0], point_3d[1], point_3d[2], &center_point_2d.x, &center_point_2d.y);
    center_point_2d.x = center_point_2d.x - voronoi->offset()[0];
    center_point_2d.y = center_point_2d.y - voronoi->offset()[1];

    // Fix points if they are out of range
    if (center_point_2d.x < 0) center_point_2d.x = 0;
    else if (center_point_2d.x >= voronoi->width()) center_point_2d.x = voronoi->width() - 1;
    if (center_point_2d.y < 0) center_point_2d.y = 0;
    else if (center_point_2d.y >= voronoi->height()) center_point_2d.y = voronoi->height() - 1;

    // Compute of the radio in pixels
    uint8_t* voronoi_value = voronoi->getRowPtr(center_point_2d.y);
    uchar voronoi_region = joint.getType() + 1;

    // Explore right radio
    int i = center_point_2d.x;
    while (i < voronoi->width() && voronoi_value[i] == voronoi_region) {
        i++;
    }
    int right_radio = i - center_point_2d.x;

    // Explore left radio
    i = center_point_2d.x;
    while (i >= 0 && voronoi_value[i] == voronoi_region) {
        i--;
    }
    int left_radio = center_point_2d.x - i;

    // Explore down radio
    i = center_point_2d.y;
    while (i < voronoi->height() && voronoi->getItem(i, center_point_2d.x) == voronoi_region) {
        i++;
    }
    int down_radio = i - center_point_2d.y;

    // Explore up radio
    i = center_point_2d.y;
    while (i >= 0 && voronoi->getItem(i, center_point_2d.x) == voronoi_region) {
        i--;
    }
    int up_radio = center_point_2d.y - i;

    float min_vertical_radio = dai::min<int>(down_radio, up_radio);
    float min_horizontal_radio = dai::min<int>(right_radio, left_radio);
    float radio_size = dai::max<float>(16.0f, dai::min<float>(min_vertical_radio, min_horizontal_radio) );

    // Create KeyPoint
    cv::KeyPoint kp(center_point_2d, /*16.0f*/ radio_size, 270.0f); // 16.0f, 32.0f
    return kp;
}

/**
 * Característica que divide la persona en dos mitades, parte superior y parte inferior, y computa para cada una de ellas
 * su histograma.
 *
 * @brief PersonReid::feature_2parts_hist
 * @param colorFrame
 * @param instance_info
 * @return
 */
DescriptorPtr PersonReid::feature_2parts_hist(shared_ptr<ColorFrame> colorFrame, const InstanceInfo& instance_info) const
{
    cv::Mat inputImg(colorFrame->height(), colorFrame->width(),
                 CV_8UC3, (void*)colorFrame->getDataPtr(), colorFrame->getStride());

    cv::Mat upper_mask, lower_mask;
    dai::computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask);

    // Color Palette with 16-16-16 levels
    //cv::Mat indexedImg = dai::convertRGB2Indexed161616(inputImg);

    // Color Palette with 8-8-4 levels
    cv::Mat indexedImg = dai::convertRGB2Indexed884(inputImg);

    // Compute the histogram for the upper (torso) and lower (leggs) parts of each frame in the buffer
    auto u_hist = Histogram1c::create(indexedImg, {0, 255}, upper_mask);
    auto l_hist = Histogram1c::create(indexedImg, {0, 255}, lower_mask);

    dai::colorImageWithMask(inputImg, inputImg, upper_mask, lower_mask);

    // Create feature
    shared_ptr<JointHistograms1c> feature = make_shared<JointHistograms1c>(instance_info, 1);
    feature->addHistogram(*u_hist);
    feature->addHistogram(*l_hist);

    return feature;
}

/**
 * Característica que utiliza distancias antropométricas basándose en el esqueleto.
 * @brief PersonReid::feature_skeleton_distances
 * @param colorFrame
 * @param skeleton
 * @param instance_info
 * @return
 */
DescriptorPtr PersonReid::feature_skeleton_distances(ColorFrame &colorFrame, Skeleton &skeleton, const InstanceInfo& instance_info) const
{
    shared_ptr<DistancesFeature> feature = make_shared<DistancesFeature>(instance_info, colorFrame.getIndex());

    SkeletonJoint spine = skeleton.getJoint(SkeletonJoint::JOINT_SPINE);
    SkeletonJoint knee_left = skeleton.getJoint(SkeletonJoint::JOINT_LEFT_KNEE);
    SkeletonJoint foot_right = skeleton.getJoint(SkeletonJoint::JOINT_RIGHT_FOOT);
    SkeletonJoint foot_left = skeleton.getJoint(SkeletonJoint::JOINT_LEFT_FOOT);
    SkeletonJoint knee_right = skeleton.getJoint(SkeletonJoint::JOINT_RIGHT_KNEE);
    SkeletonJoint hip_left = skeleton.getJoint(SkeletonJoint::JOINT_LEFT_HIP);
    SkeletonJoint hip_right = skeleton.getJoint(SkeletonJoint::JOINT_RIGHT_HIP);
    SkeletonJoint neck = skeleton.getJoint(SkeletonJoint::JOINT_CENTER_SHOULDER);
    SkeletonJoint head =  skeleton.getJoint(SkeletonJoint::JOINT_HEAD);
    SkeletonJoint shoulder_left = skeleton.getJoint(SkeletonJoint::JOINT_LEFT_SHOULDER);
    SkeletonJoint shoulder_right = skeleton.getJoint(SkeletonJoint::JOINT_RIGHT_SHOULDER);
    SkeletonJoint elbow_left = skeleton.getJoint(SkeletonJoint::JOINT_LEFT_ELBOW);
    SkeletonJoint elbow_right = skeleton.getJoint(SkeletonJoint::JOINT_RIGHT_ELBOW);
    SkeletonJoint hand_left = skeleton.getJoint(SkeletonJoint::JOINT_LEFT_HAND);
    SkeletonJoint hand_right = skeleton.getJoint(SkeletonJoint::JOINT_RIGHT_HAND);

    // Hip center
    Point3f base_line = Point3f::vector(hip_left.getPosition(), hip_right.getPosition());
    Point3f hip_center;
    hip_center[0] = hip_left.getPosition()[0] + base_line[0]/2;
    hip_center[1] = hip_left.getPosition()[1] + base_line[1]/2;
    hip_center[2] = hip_left.getPosition()[2] + base_line[2]/2;

    /*static int total_wrong_joints = 0;

    for (SkeletonJoint joint : skeleton.joints()) {
        if (joint.getPositionConfidence() == 0.0f) {
            total_wrong_joints++;
            qDebug() << "Wrong joints:" << total_wrong_joints;
        }
    }*/

    // Padilla
    // hip-head
    feature->addDistance(Point3f::euclideanDistance(hip_center, head.getPosition()));

    // neck-head
    feature->addDistance(Point3f::euclideanDistance(neck.getPosition(), head.getPosition()) );
    //feature->addDistance(Point3f::euclideanDistance(shoulder_left.getPosition(), shoulder_right.getPosition()) );

    // Hips
    float hips_dist = Point3f::euclideanDistance(hip_left.getPosition(), hip_right.getPosition());
    feature->addDistance(hips_dist);

    // l.hip-r.shoulder, r.hip-l.shoulder
    feature->addDistance(Point3f::euclideanDistance(hip_left.getPosition(), shoulder_right.getPosition()) );
    feature->addDistance(Point3f::euclideanDistance(hip_right.getPosition(), shoulder_left.getPosition()) );

    // Head
    feature->addDistance(head.getPosition()[1]);

    // neck-l.shoulder, neck-r.shoulder
    feature->addDistance(Point3f::euclideanDistance(neck.getPosition(), shoulder_left.getPosition()) );
    feature->addDistance(Point3f::euclideanDistance(neck.getPosition(), shoulder_right.getPosition()) );

    // l.shoulder-l.hip, r.shoulder-r.hip
    feature->addDistance(Point3f::euclideanDistance(shoulder_left.getPosition(), hip_left.getPosition()) );
    feature->addDistance(Point3f::euclideanDistance(shoulder_right.getPosition(), hip_right.getPosition()) );

    // legs
    float upper_leg_right_dist = Point3f::euclideanDistance(knee_right.getPosition(), hip_right.getPosition());
    float upper_leg_left_dist = Point3f::euclideanDistance(knee_left.getPosition(), hip_left.getPosition());
    float lower_leg_right_dist = Point3f::euclideanDistance(foot_right.getPosition(), knee_right.getPosition());
    float lower_leg_left_dist = Point3f::euclideanDistance(foot_left.getPosition(), knee_left.getPosition());
    feature->addDistance(upper_leg_right_dist / lower_leg_right_dist);
    feature->addDistance(upper_leg_left_dist / lower_leg_left_dist);

    // arms
    float upper_arm_right_dist = Point3f::euclideanDistance(elbow_right.getPosition(), shoulder_right.getPosition());
    float upper_arm_left_dist = Point3f::euclideanDistance(elbow_left.getPosition(), shoulder_left.getPosition());
    float lower_arm_right_dist = Point3f::euclideanDistance(hand_right.getPosition(), elbow_right.getPosition());
    float lower_arm_left_dist = Point3f::euclideanDistance(hand_left.getPosition(), elbow_left.getPosition());
    feature->addDistance(upper_arm_right_dist / lower_arm_right_dist);
    feature->addDistance(upper_arm_left_dist / lower_arm_left_dist);

    // corpulencia
    float shoulders_dist = Point3f::euclideanDistance(shoulder_left.getPosition(), shoulder_right.getPosition());
    feature->addDistance(shoulders_dist / hips_dist);

    // Munaro
    /*
    feature->addDistance(head.getPosition()[1]); // a
    feature->addDistance(neck.getPosition()[1]); // b
    feature->addDistance(Point3f::euclideanDistance(neck.getPosition(), shoulder_left.getPosition()) ); // c
    feature->addDistance(Point3f::euclideanDistance(neck.getPosition(), shoulder_right.getPosition()) ); // d
    feature->addDistance(Point3f::euclideanDistance(spine.getPosition(), shoulder_right.getPosition()) ); // e
    feature->addDistance(//f
          Point3f::euclideanDistance(shoulder_right.getPosition(), elbow_right.getPosition()) +
          Point3f::euclideanDistance(elbow_right.getPosition(), hand_right.getPosition()));
    feature->addDistance(//g
          Point3f::euclideanDistance(shoulder_left.getPosition(), elbow_left.getPosition()) +
          Point3f::euclideanDistance(elbow_left.getPosition(), hand_left.getPosition()));
    feature->addDistance(Point3f::euclideanDistance(knee_right.getPosition(), hip_right.getPosition())); // h
    feature->addDistance(Point3f::euclideanDistance(knee_left.getPosition(), hip_left.getPosition())); // i
    feature->addDistance(Point3f::euclideanDistance(spine.getPosition(), neck.getPosition())); // j
    feature->addDistance(Point3f::euclideanDistance(hip_left.getPosition(), hip_right.getPosition())); // k
    feature->addDistance( // l
          Point3f::euclideanDistance(spine.getPosition(), neck.getPosition()) /
          Point3f::euclideanDistance(knee_right.getPosition(), hip_right.getPosition()));
    feature->addDistance( // m
          Point3f::euclideanDistance(spine.getPosition(), neck.getPosition()) /
          Point3f::euclideanDistance(knee_left.getPosition(), hip_left.getPosition()));*/

    // Barbosa
    /*float mean_shoulder = (Point3f::euclideanDistance(neck.getPosition(), shoulder_left.getPosition()) + Point3f::euclideanDistance(neck.getPosition(), shoulder_right.getPosition())) / 2.0f;
    float mean_hip = (hip_left.getPosition()[2] + hip_right.getPosition()[2]) / 2.0f;
    float d1 = 1.0f / Point3f::euclideanDistance(Point3f(0.0, 0.0, 0.0), head.getPosition());

    feature->addDistance(Point3f::euclideanDistance(Point3f(0.0, 0.0, 0.0), head.getPosition()) );
    feature->addDistance(mean_shoulder / mean_hip * d1);
    feature->addDistance(head.getPosition()[1]);
    feature->addDistance(Point3f::euclideanDistance(Point3f(0.0, 0.0, 0.0), neck.getPosition()) );
    feature->addDistance(Point3f::euclideanDistance(neck.getPosition(), shoulder_left.getPosition()) );
    feature->addDistance(Point3f::euclideanDistance(neck.getPosition(), shoulder_right.getPosition()) );
    feature->addDistance(Point3f::euclideanDistance(spine.getPosition(), shoulder_right.getPosition()) );*/

    return feature;
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

// Extract Voronoi cells as a mask
shared_ptr<MaskFrame> PersonReid::getVoronoiCells(const DepthFrame& depthFrame, const MaskFrame& maskFrame, const Skeleton& skeleton)
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
                Point3f point(0.0f, 0.0f, float(depth[j]));
                depthFrame.convertCoordinatesToWorld(j, i, depth[j], &point[0], &point[1]);
                SkeletonJoint closerJoint = getCloserJoint(point, joints);
                mask[j] = closerJoint.getType()+1; // 0 means no user, and joint type starts at 0, so I increase it by one
            }
        }
    }

    return result;
}

shared_ptr<MaskFrame> PersonReid::getVoronoiCellsParallel(const DepthFrame& depthFrame, const MaskFrame& maskFrame, const Skeleton& skeleton)
{
    Q_ASSERT(depthFrame.width() == maskFrame.width() && depthFrame.height() == maskFrame.height());

    //QElapsedTimer timer;
    //timer.start();

    shared_ptr<MaskFrame> output_mask = static_pointer_cast<MaskFrame>(maskFrame.clone());
    QList<SkeletonJoint> joints = skeleton.joints();

    auto code = [depthFrame, joints, &output_mask](int row) -> void
    {
        uint16_t* depth = depthFrame.getRowPtr(row);
        uint8_t* mask = output_mask->getRowPtr(row);

        for (int j=0; j<depthFrame.width(); ++j)
        {
            if (mask[j] > 0)
            {
                Point3f point(0.0f, 0.0f, float(depth[j]));
                depthFrame.convertCoordinatesToWorld(j, row, depth[j], &point[0], &point[1]);
                SkeletonJoint closerJoint = PersonReid::getCloserJoint(point, joints);
                mask[j] = closerJoint.getType()+1;
            }
        }
    };

    std::vector<std::future<void>> workers;

    for (int i = 0; i < depthFrame.height(); i++)
    {
        workers.push_back( std::async(code, i) );
    }

    for (std::future<void>& f : workers)
        f.wait();

    //qDebug() << "Voronoi Cells" << timer.elapsed();

    return output_mask;
}

void PersonReid::drawSkeleton(ColorFrame& colorFrame, const Skeleton& skeleton) const
{
    // Draw Joints
    for (const SkeletonJoint& joint : skeleton.joints()) {
        float x, y;
        skeleton.convertCoordinatesToDepth(joint.getPosition()[0],
                joint.getPosition()[1], joint.getPosition()[2], &x, &y);

        if (joint.getType() > SkeletonJoint::JOINT_USER_RESERVED)
            drawPoint(colorFrame, x - colorFrame.offset()[0], y - colorFrame.offset()[1], {255, 255, 0});
        else
            drawPoint(colorFrame, x - colorFrame.offset()[0], y - colorFrame.offset()[1], {0, 255, 255});
    }
}

SkeletonJoint PersonReid::getCloserJoint(const Point3f& cloudPoint, const QList<SkeletonJoint>& joints)
{
    SkeletonJoint minJoint;
    float minDistance = numeric_limits<float>::max();

    for (const SkeletonJoint& joint : joints)
    {
        float distance = Point3f::euclideanDistanceSquared(cloudPoint, joint.getPosition());

        if (distance < minDistance) {
            minDistance = distance;
            minJoint = joint; // copy
        }
    }

    return minJoint;
}

void PersonReid::colorImageWithVoronoid(ColorFrame& colorFrame, MaskFrame& voronoi)
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
            /*else {
                pixel[j].red = 0;
                pixel[j].green = 0;
                pixel[j].blue = 0;
            }*/
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

void PersonReid::highLightDepth(ColorFrame &colorFrame, DepthFrame &depthFrame) const
{
    Q_ASSERT(colorFrame.width() == depthFrame.width() && colorFrame.height() == depthFrame.height());

    QMap<uint16_t, float> colorHistogram;
    DepthFrame::calculateHistogram(colorHistogram, depthFrame);

    for (int i=0; i<depthFrame.height(); ++i)
    {
        const uint16_t* pDepth = depthFrame.getRowPtr(i);
        RGBColor* pColor = colorFrame.getRowPtr(i);

        for (int j=0; j<depthFrame.width(); ++j)
        {
           uint8_t color = colorHistogram[pDepth[j]];
           pColor[j].red = color;
           pColor[j].green = color;
           pColor[j].blue = 0;
        }
    }
}

/*void PersonReid::printClusters(const QList<Cluster<Descriptor> > &clusters) const
{
    int i = 0;

    for (Cluster<Descriptor> cluster : clusters)
    {
        if (!cluster.samples.isEmpty())
        {
            DescriptorPtr centroid = cluster.centroid;
            printf("Cluster %i (size=%i, actor=%i %i)\n", i+1, cluster.samples.size(), centroid->label().getActor(), centroid->label().getSample());

            for (DescriptorPtr items : cluster.samples) {
                const InstanceInfo& label = items->label();
                printf("(%i %i) ", label.getActor(), label.getSample());
            }

            cout << endl << endl;
            ++i;
        }
    }
}*/

void PersonReid::makeUpJoints(Skeleton& skeleton, bool only_middle_points)
{
    const Skeleton::SkeletonLimb* limbMap = skeleton.getLimbsMap();
    int user_joint_id = 1;

    for (int i=0; i<skeleton.getLimbsCount(); ++i)
    {
        SkeletonJoint joint1 = skeleton.getJoint(limbMap[i].joint1);
        SkeletonJoint joint2 = skeleton.getJoint(limbMap[i].joint2);

        // Compute middle point between two joints
        Point3f vector = Point3f::vector(joint1.getPosition(), joint2.getPosition());
        Point3f medium_point;
        medium_point[0] = joint1.getPosition()[0] + vector[0]/2;
        medium_point[1] = joint1.getPosition()[1] + vector[1]/2;
        medium_point[2] = joint1.getPosition()[2] + vector[2]/2;

        if (only_middle_points) {
            skeleton.setJoint( (SkeletonJoint::JointType) (SkeletonJoint::JOINT_USER_RESERVED + user_joint_id),
                               SkeletonJoint(medium_point, joint1.getType()));
            user_joint_id++;
        }
        else {
            // Compute middle point between previous middle point and joint2
            vector = Point3f::vector(medium_point, joint2.getPosition());
            Point3f near_joint2_point;
            near_joint2_point[0] = medium_point[0] + vector[0]/2;
            near_joint2_point[1] = medium_point[1] + vector[1]/2;
            near_joint2_point[2] = medium_point[2] + vector[2]/2;

            // Compute middle point between previous middle point and joint1
            vector = Point3f::vector(joint1.getPosition(), medium_point);
            Point3f near_joint1_point;
            near_joint1_point[0] = joint1.getPosition()[0] + vector[0]/2;
            near_joint1_point[1] = joint1.getPosition()[1] + vector[1]/2;
            near_joint1_point[2] = joint1.getPosition()[2] + vector[2]/2;

            // Add two new joints to the Skeleotn
            skeleton.setJoint( (SkeletonJoint::JointType) (SkeletonJoint::JOINT_USER_RESERVED + user_joint_id),
                               SkeletonJoint(near_joint2_point, joint2.getType()));

            skeleton.setJoint( (SkeletonJoint::JointType) (SkeletonJoint::JOINT_USER_RESERVED + user_joint_id+1),
                               SkeletonJoint(near_joint1_point, joint1.getType()));

            user_joint_id += 2;
        }
    }
}

void PersonReid::makeUpOnlySomeJoints(Skeleton& skeleton)
{
    int user_joint_id = 1;

    SkeletonJoint knee_right = skeleton.getJoint(SkeletonJoint::JOINT_RIGHT_KNEE);
    SkeletonJoint hip_right  = skeleton.getJoint(SkeletonJoint::JOINT_RIGHT_HIP);
    skeleton.setJoint( (SkeletonJoint::JointType) (SkeletonJoint::JOINT_USER_RESERVED + user_joint_id++),
                       SkeletonJoint(Point3f::middlePoint(knee_right.getPosition(), hip_right.getPosition()), knee_right.getType()));

    SkeletonJoint knee_left  = skeleton.getJoint(SkeletonJoint::JOINT_LEFT_KNEE);
    SkeletonJoint hip_left   = skeleton.getJoint(SkeletonJoint::JOINT_LEFT_HIP);
    skeleton.setJoint( (SkeletonJoint::JointType) (SkeletonJoint::JOINT_USER_RESERVED + user_joint_id++),
                       SkeletonJoint(Point3f::middlePoint(knee_left.getPosition(), hip_left.getPosition()), knee_left.getType()));

    /*SkeletonJoint shoulder_right = skeleton.getJoint(SkeletonJoint::JOINT_RIGHT_SHOULDER);
    skeleton.setJoint( (SkeletonJoint::JointType) (SkeletonJoint::JOINT_USER_RESERVED + user_joint_id++),
                       SkeletonJoint(Point3f::middlePoint(shoulder_right.getPosition(), hip_right.getPosition()), SkeletonJoint::JOINT_SPINE));

    SkeletonJoint shoulder_left = skeleton.getJoint(SkeletonJoint::JOINT_LEFT_SHOULDER);
    skeleton.setJoint( (SkeletonJoint::JointType) (SkeletonJoint::JOINT_USER_RESERVED + user_joint_id++),
                       SkeletonJoint(Point3f::middlePoint(shoulder_left.getPosition(), hip_left.getPosition()), SkeletonJoint::JOINT_SPINE));*/
}



void PersonReid::drawPoint(ColorFrame& colorFrame, int x, int y, RGBColor color) const
{
    if (x >= 1 && (x+1) < colorFrame.width() && y >= 1 && (y+1) < colorFrame.height())
    {
        shared_ptr<ColorFrame> subColor = colorFrame.subFrame(y-1, x-1, 3, 3);

        for (int i=0; i<subColor->height(); ++i)
        {
            RGBColor* pixel = subColor->getRowPtr(i);

            for (int j=0; j<subColor->width(); ++j)
            {
                pixel[j] = color;
            }
        }
    }
}


} // End Namespace
