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
#include "viewer/InstanceViewerWindow.h"
#include "Config.h"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv_utils.h"
#include "types/Histogram.h"
#include <QThread>
#include <QImage>
#include <QFile>
#include <future>
#include <QCryptographicHash>
#include "JointHistograms.h"
#include "DistancesFeature.h"
#include "RegionDescriptor.h"
#include "DescriptorSet.h"


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


// Test save image and read it
void PersonReid::test1()
{
    cv::Mat color_mat = cv::imread("C:/prueba.png");

    shared_ptr<ColorFrame> color_f = make_shared<ColorFrame>();
    color_f->setDataPtr(color_mat.cols, color_mat.rows, (RGBColor*) color_mat.data, color_mat.step);

    // Save Image
    QByteArray buffer = color_f->toBinary();
    QFile colorFile("file.bin");
    colorFile.open(QIODevice::WriteOnly);
    colorFile.write(buffer);
    colorFile.close();
    buffer.clear();

    // Read Image
    colorFile.open(QIODevice::ReadOnly);
    buffer = colorFile.readAll();
    shared_ptr<ColorFrame> color_copy = make_shared<ColorFrame>();
    color_copy->loadData(buffer);

    InstanceViewerWindow viewer;
    viewer.show();
    viewer.showFrame(color_copy);

    cv::imshow("prueba", color_mat);
    cv::waitKey(5000);
}

void PersonReid::test2()
{
    // Read Image
    QFile colorFile("file.bin");
    colorFile.open(QIODevice::ReadOnly);
    QByteArray buffer = colorFile.readAll();
    shared_ptr<ColorFrame> colorFrame = make_shared<ColorFrame>();
    colorFrame->loadData(buffer);

    // Show
    cv::Mat color_mat(colorFrame->height(), colorFrame->width(), CV_8UC3, (void*) colorFrame->getDataPtr(), colorFrame->getStride());
    cv::imshow("prueba", color_mat);
    cv::waitKey(5000);
}

void PersonReid::test3()
{
    Dataset* dataset = new DAI4REID_Parsed;
    dataset->setPath("C:/datasets/DAI4REID/parse_subset");
    const DatasetMetadata& metadata = dataset->getMetadata();

    //shared_ptr<InstanceInfo> instance_info = metadata.instance(1, 1, 302, QList<QString>());

    QList<shared_ptr<InstanceInfo>> instances = metadata.instances({1,2,3,4,5},
                                                                   {2},
                                                                   DatasetMetadata::ANY_LABEL);
    // Create container for read frames
    QHashDataFrames readFrames;
    QCryptographicHash sha1_hash(QCryptographicHash::Sha1);

    for (shared_ptr<InstanceInfo> instance_info : instances)
    {
        std::string fileName = instance_info->getFileName(DataFrame::Color).toStdString();

        printf("actor %i sample %i file %s\n", instance_info->getActor(),
               instance_info->getSample(),
               fileName.c_str());

        fflush(stdout);

        // Get Sample
        shared_ptr<StreamInstance> instance = dataset->getInstance(*instance_info, DataFrame::Color);

        // Open Instances
        instance->open();

        // Read frames
        instance->readNextFrame(readFrames);

        // Get Frames
        auto colorFrame = static_pointer_cast<ColorFrame>(readFrames.value(DataFrame::Color));
        auto depthFrame = static_pointer_cast<DepthFrame>(readFrames.value(DataFrame::Depth));
        auto maskFrame = static_pointer_cast<MaskFrame>(readFrames.value(DataFrame::Mask));
        auto skeletonFrame = static_pointer_cast<SkeletonFrame>(readFrames.value(DataFrame::Skeleton));
        colorFrame->setOffset(depthFrame->offset());

        // Process
        QList<int> users = skeletonFrame->getAllUsersId();
        shared_ptr<Skeleton> skeleton = skeletonFrame->getSkeleton(users.at(0));

        sha1_hash.addData(colorFrame->toBinary());
        sha1_hash.addData(depthFrame->toBinary());
        sha1_hash.addData(maskFrame->toBinary());
        sha1_hash.addData(skeleton->toBinary());

        // Close Instances
        instance->close();
    }

    qDebug() << sha1_hash.result().toHex();
}

void PersonReid::execute()
{
    //parseDataset();

    // Select dataset
    //Dataset* dataset = new IASLAB_RGBD_ID;
    //dataset->setPath("/Volumes/Files/Datasets/IASLAB-RGBD-ID");
    Dataset* dataset = new DAI4REID_Parsed;
    dataset->setPath("/Volumes/Files/Datasets/DAI4REID_Parsed");

    // Select actors
    //QList<int> actors = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    QList<int> actors = {1, 2, 3, 4, 5};

    // Start
    QVector<float> results(actors.size());
    int num_tests = 0;

    // Train cam1, Test cam2 and viceversa
    for (int i=0; i<2; ++i)
    {
        // Training (camera 1)
        QList<DescriptorPtr> gallery = train(dataset, actors, i==0 ? 1 : 2);

        for (DescriptorPtr target : gallery) {
            qDebug() << target->label().getActor() << target->label().getSample();
        }

        // Validation (camera 2)
        validate(dataset, actors, i==0 ? 2 : 1, gallery, results, &num_tests);
    }

    // Show Results
    normalise_results(results, num_tests);
    print_results(results);

    // Quit
    QCoreApplication::instance()->quit();
}

QList<DescriptorPtr> PersonReid::train(Dataset* dataset, QList<int> actors, int camera)
{
    const DatasetMetadata& metadata = dataset->getMetadata();
    QList<DescriptorPtr> actor_samples;
    QList<DescriptorPtr> centroids;

    // Create container for read frames
    QHashDataFrames readFrames;

    // For each actor, compute the feature that minimises the distance to each other sample of
    // the same actor.
    for (int actor : actors)
    {
        QList<shared_ptr<InstanceInfo>> instances = metadata.instances({actor},
                                                                       {camera},
                                                                       DatasetMetadata::ANY_LABEL);

        for (shared_ptr<InstanceInfo> instance_info : instances)
        {
            QElapsedTimer timer;
            timer.start();

            // Get Sample
            shared_ptr<StreamInstance> instance = dataset->getInstance(*instance_info, DataFrame::Color);

            // Open Instances
            instance->open();

            // Read frames
            instance->readNextFrame(readFrames);

            // Get Frames
            auto colorFrame = static_pointer_cast<ColorFrame>(readFrames.value(DataFrame::Color));
            auto depthFrame = static_pointer_cast<DepthFrame>(readFrames.value(DataFrame::Depth));
            auto maskFrame = static_pointer_cast<MaskFrame>(readFrames.value(DataFrame::Mask));
            auto skeletonFrame = static_pointer_cast<SkeletonFrame>(readFrames.value(DataFrame::Skeleton));
            //colorFrame->setOffset(depthFrame->offset());

            // Process
            QList<int> users = skeletonFrame->getAllUsersId();
            shared_ptr<Skeleton> skeleton = skeletonFrame->getSkeleton(users.at(0));
            //highLightMask(*colorFrame, *maskFrame);
            //highLightDepth(*colorFrame, *depthFrame);
            //drawJoints(*colorFrame, skeleton->joints());
            DescriptorPtr feature = feature_joints_hist(*colorFrame, *depthFrame, *maskFrame, *skeleton, *instance_info);
            //DescriptorPtr feature = feature_region_descriptor(*colorFrame, *depthFrame, *maskFrame, *skeleton, *instance_info);
            //DescriptorPtr feature = feature_skeleton_distances(*colorFrame, *skeleton, *instance_info);
            //DescriptorPtr feature = feature_joint_descriptor(*colorFrame, *skeleton, *instance_info);
            //DescriptorPtr feature = feature_fusion(*colorFrame, *depthFrame, *maskFrame, *skeleton, *instance_info);

            //qDebug() << feature->sizeInBytes();

            if (feature) {
                actor_samples << feature;
                //samples << feature;
            }

            // Show
            //show_images(colorFrame, maskFrame, depthFrame, skeleton);

            // Close Instances
            instance->close();

            qDebug("actor %i sample %i fps %f", instance_info->getActor(), instance_info->getSample(), 1000.0f / timer.elapsed());
        }

        qDebug() << "Actor samples" << actor_samples.size();

        // Learn Minimum: Use the signature that minimise its distance to all of the other signatures for the same actor
        DescriptorPtr selectedFeature = Descriptor::minFeatureParallel(actor_samples);
        centroids << selectedFeature;
        qDebug("Init. Centroid %i = actor %i %i %i", actor, selectedFeature->label().getActor(),
                                                       selectedFeature->label().getSample(),
                                                       selectedFeature->frameId());

        // Learn K features for the same actor (K=3)
        /*qDebug() << "Computing K-Means";
        auto kmeans = KMeans<Descriptor>::execute(actor_samples, 5, 5);

        foreach (auto cluster, kmeans->getClusters()) {
            gallery << cluster.centroid;
        }*/

        actor_samples.clear();
    }

    // Learn A: Learning a signature is the same as clustering the input data into num_actor sets
    // and use the centroid of each cluster as model.
    /*qDebug() << "Computing K-Means";
    auto kmeans = KMeans<Feature>::execute(samples, actors.size(), centroids);
    printClusters(kmeans->getClusters());

    QList<shared_ptr<Feature>> gallery;

    foreach (auto cluster, kmeans->getClusters()) {
        gallery << cluster.centroid;
    }*/

    // Learn B: Use the signature that minimises the distance to the rest of signatures
    // of each actor.
    QList<DescriptorPtr> gallery = centroids;

    return gallery;
}

void PersonReid::validate(Dataset* dataset, const QList<int> &actors, int camera, const QList<DescriptorPtr>& gallery, QVector<float>& results, int *num_tests)
{
    const DatasetMetadata& metadata = dataset->getMetadata();
    int total_tests = 0;

    QList<shared_ptr<InstanceInfo>> instances = metadata.instances(actors,
                                                                   {camera},
                                                                   DatasetMetadata::ANY_LABEL);
    // Create container for read frames
    QHashDataFrames readFrames;

    // Start validation
    for (shared_ptr<InstanceInfo> instance_info : instances)
    {
        std::string fileName = instance_info->getFileName(DataFrame::Color).toStdString();

        /*qDebug("actor %i sample %i file %s", instance_info->getActor(),
               instance_info->getSample(),
               fileName.c_str());*/

        // Get Sample
        shared_ptr<StreamInstance> instance = dataset->getInstance(*instance_info, DataFrame::Color);

        // Open Instances
        instance->open();

        // Read frames
        instance->readNextFrame(readFrames);

        // Get Frames
        auto colorFrame = static_pointer_cast<ColorFrame>(readFrames.value(DataFrame::Color));
        auto depthFrame = static_pointer_cast<DepthFrame>(readFrames.value(DataFrame::Depth));
        auto maskFrame = static_pointer_cast<MaskFrame>(readFrames.value(DataFrame::Mask));
        auto skeletonFrame = static_pointer_cast<SkeletonFrame>(readFrames.value(DataFrame::Skeleton));
        colorFrame->setOffset(depthFrame->offset());

        // Process
        QList<int> users = skeletonFrame->getAllUsersId();
        shared_ptr<Skeleton> skeleton = skeletonFrame->getSkeleton(users.at(0));

        DescriptorPtr query = feature_joints_hist(*colorFrame, *depthFrame, *maskFrame, *skeleton, *instance_info);
        //DescriptorPtr query = feature_region_descriptor(*colorFrame, *depthFrame, *maskFrame, *skeleton, *instance_info);
        //DescriptorPtr query = feature_skeleton_distances(*colorFrame, *skeleton, *instance_info);
        //DescriptorPtr query = feature_joint_descriptor(*colorFrame, *skeleton, *instance_info);
        //DescriptorPtr query = feature_fusion(*colorFrame, *depthFrame, *maskFrame, *skeleton, *instance_info);

        if (query) {
            // CMC
            QMap<float, int> query_results = compute_distances_to_all_samples(*query, gallery);
            int pos = cummulative_match_curve(query_results, results, query->label().getActor());
            qDebug("Results for actor %i sample %i file %s (pos=%i)", instance_info->getActor(), instance_info->getSample(), fileName.c_str(), pos+1);
            print_query_results(query_results, pos);
            qDebug() << "--------------------------";
            total_tests++;
        }

        // Close Instances
        instance->close();
    }

    if (num_tests) {
        *num_tests += total_tests;
    }
}

void PersonReid::parseDataset()
{
    Dataset* dataset = new DAI4REID;
    dataset->setPath("/files/DAI4REID");
    const DatasetMetadata& metadata = dataset->getMetadata();

    // Create memory for all kind of frames
    QHashDataFrames readFrames = allocateMemory();

    // For each actor, parse his/her samples
    QList<int> actors = {1,2,3,4,5};

    for (int actor : actors)
    {
        QList<shared_ptr<InstanceInfo>> instances_md = metadata.instances({actor},
                                                                       {2},
                                                                       DatasetMetadata::ANY_LABEL);
        shared_ptr<InstanceInfo> instance_info = instances_md.at(0);
        std::string fileName = instance_info->getFileName(DataFrame::Color).toStdString();

        printf("actor %i sample %i file %s\n", instance_info->getActor(),
               instance_info->getSample(),
               fileName.c_str());

        std::fflush(stdout);

        // Get instances
        QList<shared_ptr<StreamInstance>> instances;
        instances << dataset->getInstance(*instance_info, DataFrame::Color);
        instances << dataset->getInstance(*instance_info, DataFrame::Metadata);

        // Open Instances
        for (shared_ptr<StreamInstance> instance : instances) {
            instance->open();
        }

        shared_ptr<OpenNIColorInstance> colorInstance = static_pointer_cast<OpenNIColorInstance>(instances.at(0));
        colorInstance->device().playbackControl()->setSpeed(0.15f);

        // Read frames
        uint previousFrame = 0;

        while (colorInstance->hasNext())
        {
            for (shared_ptr<StreamInstance> instance : instances) {
                instance->readNextFrame(readFrames);
            }

            // Get Frames
            auto colorFrame = static_pointer_cast<ColorFrame>(readFrames.value(DataFrame::Color));
            auto depthFrame = static_pointer_cast<DepthFrame>(readFrames.value(DataFrame::Depth));
            auto maskFrame = static_pointer_cast<MaskFrame>(readFrames.value(DataFrame::Mask));
            auto skeletonFrame = static_pointer_cast<SkeletonFrame>(readFrames.value(DataFrame::Skeleton));
            auto metadataFrame = static_pointer_cast<MetadataFrame>(readFrames.value(DataFrame::Metadata));

            if (previousFrame + 1 != colorFrame->getIndex())
                qDebug() << "Frame Skip" << colorFrame->getIndex();

            qDebug() << "Processing frame" << colorFrame->getIndex();

            // Process
            QList<int> users = skeletonFrame->getAllUsersId();

            // Work with the user inside of the Bounding Box
            if (!users.isEmpty() && !metadataFrame->boundingBoxes().isEmpty() /*&& colorFrame->getIndex() > 140*/)
            {
                int firstUser = users.at(0);
                shared_ptr<Skeleton> skeleton = skeletonFrame->getSkeleton(firstUser);

                // Get ROIs
                BoundingBox bb = metadataFrame->boundingBoxes().first();
                shared_ptr<ColorFrame> roiColor = colorFrame->subFrame(bb);
                shared_ptr<DepthFrame> roiDepth = depthFrame->subFrame(bb);
                shared_ptr<MaskFrame> roiMask = maskFrame->subFrame(bb);

                QString fileName = "U" + QString::number(instance_info->getActor()) +
                                  "_C" + QString::number(instance_info->getCamera()) +
                                  "_F" + QString::number(roiColor->getIndex());

                // Show
                cv::Mat color_mat(roiColor->height(), roiColor->width(), CV_8UC3,
                                  (void*) roiColor->getDataPtr(), roiColor->getStride());

                cv::imshow("img1", color_mat);
                cv::waitKey(1);
                QCoreApplication::processEvents();

                // Save color as JPEG
                QImage image( (uchar*) roiColor->getDataPtr(), roiColor->width(), roiColor->height(),
                             roiColor->getStride(), QImage::Format_RGB888);
                image.save("data/" + fileName + "_color.jpg");

                // Save depth
                /*cv::Mat depth_mat(roiDepth->height(), roiDepth->width(), CV_16UC1,
                                  (uchar*) roiDepth->getDataPtr(), roiDepth->getStride());
                cv::imwrite(QString("data/" + fileName + "_depth.png").toStdString(), depth_mat);*/
                QFile depthFile("data/" + fileName + "_depth.bin");
                QByteArray depth_data = roiDepth->toBinary();
                depthFile.open(QIODevice::WriteOnly);
                depthFile.write(depth_data);
                depthFile.close();

                // Save mask
                QFile maskFile("data/" + fileName + "_mask.bin");
                QByteArray mask_data = roiMask->toBinary();
                maskFile.open(QIODevice::WriteOnly);
                maskFile.write(mask_data);
                maskFile.close();

                // Save Skeleton
                QFile file("data/" + fileName + "_skel.bin");
                QByteArray skel_data = skeleton->toBinary();
                file.open(QIODevice::WriteOnly);
                file.write(skel_data);
                file.close();
            }

            previousFrame = colorFrame->getIndex();
        }

        // Close Instances
        for (shared_ptr<StreamInstance> instance : instances) {
            instance->close();
        }
    }

    qDebug() << "Parse Finished!";
}

// Test feature of n-parts localised histograms (corresponding to joints) with OpenNI
void PersonReid::test_DAI4REID()
{
    QList<int> actors = {1, 2, 3, 4, 5};

    // Training
    QList<DescriptorPtr> gallery = train_DAI4REID(actors);

    // Validation
    int num_tests = 0;
    QVector<float> results = validate_DAI4REID(actors, gallery, &num_tests);

    // Show Results
    normalise_results(results, num_tests);
    print_results(results);
}

QList<DescriptorPtr> PersonReid::train_DAI4REID(QList<int> actors)
{
    Dataset* dataset = new DAI4REID;
    dataset->setPath("C:/datasets/DAI4REID");
    const DatasetMetadata& metadata = dataset->getMetadata();

    InstanceViewerWindow viewer;
    viewer.show();

    QList<DescriptorPtr> samples;
    QList<DescriptorPtr> actor_samples;
    QList<DescriptorPtr> centroids;

    // Create memory for colorFrame
    QHashDataFrames readFrames = allocateMemory();

    // For each actor, compute the feature that minimises the distance to each other sample of
    // the same actor.
    for (int actor : actors)
    {
        QList<shared_ptr<InstanceInfo>> instances = metadata.instances({actor},
                                                                       {2},
                                                                       DatasetMetadata::ANY_LABEL);

        for (shared_ptr<InstanceInfo> instance_info : instances)
        {
            std::string fileName = instance_info->getFileName(DataFrame::Color).toStdString();

            printf("actor %i sample %i file %s\n", instance_info->getActor(),
                   instance_info->getSample(),
                   fileName.c_str());

            std::fflush(stdout);

            // Get Sample
            QList<shared_ptr<StreamInstance>> instances;
            instances << dataset->getInstance(*instance_info, DataFrame::Color);
            //instances << dataset->getInstance(*instance_info, DataFrame::Depth);     // I should FIX this
            //instances << dataset->getInstance(*instance_info, DataFrame::Skeleton);  // because I don't need
            //instances << dataset->getInstance(*instance_info, DataFrame::Mask);      // to know imp. details
            instances << dataset->getInstance(*instance_info, DataFrame::Metadata);

            // Open Instances
            for (shared_ptr<StreamInstance> instance : instances) {
                instance->open();
            }

            shared_ptr<OpenNIColorInstance> colorInstance = static_pointer_cast<OpenNIColorInstance>(instances.at(0));
            colorInstance->device().playbackControl()->setSpeed(1.0f);
            m_device = &(colorInstance->device());

            // Read frames
            while (colorInstance->hasNext())
            {
                for (shared_ptr<StreamInstance> instance : instances) {
                    instance->readNextFrame(readFrames);
                }

                // Get Frames
                auto colorFrame = static_pointer_cast<ColorFrame>(readFrames.value(DataFrame::Color));
                auto depthFrame = static_pointer_cast<DepthFrame>(readFrames.value(DataFrame::Depth));
                auto maskFrame = static_pointer_cast<MaskFrame>(readFrames.value(DataFrame::Mask));
                auto skeletonFrame = static_pointer_cast<SkeletonFrame>(readFrames.value(DataFrame::Skeleton));
                auto metadataFrame = static_pointer_cast<MetadataFrame>(readFrames.value(DataFrame::Metadata));

                // Process
                QList<int> users = skeletonFrame->getAllUsersId();

                // Work with the user inside of the Bounding Box
                if (!users.isEmpty() && !metadataFrame->boundingBoxes().isEmpty() && colorFrame->getIndex() > 140)
                {
                    int firstUser = users.at(0);
                    shared_ptr<Skeleton> skeleton = skeletonFrame->getSkeleton(firstUser);

                    //drawJoints(*colorFrame, skeleton->joints());

                    // Get ROIs
                    BoundingBox bb = metadataFrame->boundingBoxes().first();
                    shared_ptr<ColorFrame> roiColor = colorFrame->subFrame(bb);
                    shared_ptr<DepthFrame> roiDepth = depthFrame->subFrame(bb);
                    shared_ptr<MaskFrame> roiMask = maskFrame->subFrame(bb);

                    DescriptorPtr feature = feature_joints_hist(*roiColor, *roiDepth, *roiMask, *skeleton,
                                                                      *instance_info);
                    if (feature) {
                        actor_samples << feature;
                        samples << feature;
                    }
                }

                // Show
                viewer.showFrame(colorFrame);
                QCoreApplication::processEvents();
            }

            // Close Instances
            for (shared_ptr<StreamInstance> instance : instances) {
                instance->close();
            }
        }

        qDebug() << "Actor samples" << actor_samples.size();
        DescriptorPtr selectedFeature = Descriptor::minFeature(actor_samples);
        centroids << selectedFeature;
        printf("Init. Centroid %i = actor %i %i %i\n", actor, selectedFeature->label().getActor(),
                                                       selectedFeature->label().getSample(),
                                                       selectedFeature->frameId());
        actor_samples.clear();
        std::fflush(stdout);
    }

    // Learn A: Learning a signature is the same as clustering the input data into num_actor sets
    // and use the centroid of each cluster as model.
    auto kmeans = KMeans<Descriptor>::execute(samples, actors.size(), centroids);
    printClusters(kmeans->getClusters());

    QList<DescriptorPtr> gallery;

    for (auto cluster : kmeans->getClusters()) {
        gallery << cluster.centroid;
    }

    // Learn B: Use the signature that minimises the distance to the rest of signatures
    // of each actor.
    //QList<shared_ptr<Feature>> gallery = centroids;

    return gallery;
}

QVector<float> PersonReid::validate_DAI4REID(const QList<int> &actors, const QList<DescriptorPtr>& gallery, int *num_tests)
{
    Dataset* dataset = new DAI4REID;
    dataset->setPath("C:/datasets/DAI4REID");
    const DatasetMetadata& metadata = dataset->getMetadata();
    int total_tests = 0;

    QList<shared_ptr<InstanceInfo>> instances = metadata.instances(actors,
                                                                   {2},
                                                                   DatasetMetadata::ANY_LABEL);


    // Create memory for colorFrame
    QHashDataFrames readFrames = allocateMemory();

    // Start validation
    QVector<float> results(actors.size());

    for (shared_ptr<InstanceInfo> instance_info : instances)
    {
        // Get Sample
        QList<shared_ptr<StreamInstance>> instances;
        instances << dataset->getInstance(*instance_info, DataFrame::Color);
        //instances << dataset->getInstance(*instance_info, DataFrame::Depth);     // I should FIX this
        //instances << dataset->getInstance(*instance_info, DataFrame::Skeleton);  // because I don't need
        //instances << dataset->getInstance(*instance_info, DataFrame::Mask);      // to know imp. details
        instances << dataset->getInstance(*instance_info, DataFrame::Metadata);

        // Open Instances
        for (shared_ptr<StreamInstance> instance : instances) {
            instance->open();
        }

        shared_ptr<OpenNIColorInstance> colorInstance = static_pointer_cast<OpenNIColorInstance>(instances.at(0));
        colorInstance->device().playbackControl()->setSpeed(0.25f);
        m_device = &(colorInstance->device());

        // Read frames
        while (colorInstance->hasNext())
        {
            for (shared_ptr<StreamInstance> instance : instances) {
                instance->readNextFrame(readFrames);
            }

            // Get Frames
            auto colorFrame = static_pointer_cast<ColorFrame>(readFrames.value(DataFrame::Color));
            auto depthFrame = static_pointer_cast<DepthFrame>(readFrames.value(DataFrame::Depth));
            auto maskFrame = static_pointer_cast<MaskFrame>(readFrames.value(DataFrame::Mask));
            auto skeletonFrame = static_pointer_cast<SkeletonFrame>(readFrames.value(DataFrame::Skeleton));
            auto metadataFrame = static_pointer_cast<MetadataFrame>(readFrames.value(DataFrame::Metadata));

            // Feature extraction
            QList<int> users = skeletonFrame->getAllUsersId();

            // Work with the user inside of the Bounding Box
            if (!users.isEmpty() && !metadataFrame->boundingBoxes().isEmpty() && colorFrame->getIndex() > 140)
            {
                int firstUser = users.at(0);
                shared_ptr<Skeleton> skeleton = skeletonFrame->getSkeleton(firstUser);

                // Get ROIs
                BoundingBox bb = metadataFrame->boundingBoxes().first();
                shared_ptr<ColorFrame> roiColor = colorFrame->subFrame(bb);
                shared_ptr<DepthFrame> roiDepth = depthFrame->subFrame(bb);
                shared_ptr<MaskFrame> roiMask = maskFrame->subFrame(bb);

                DescriptorPtr query = feature_joints_hist(*roiColor, *roiDepth, *roiMask, *skeleton,
                                                                  *instance_info);
                if (query) {
                    // CMC
                    QMap<float, int> query_results = compute_distances_to_all_samples(*query, gallery);
                    int pos = cummulative_match_curve(query_results, results, query->label().getActor());
                    cout << "Results for actor " << instance_info->getActor() << " " << instance_info->getSample() << endl;
                    print_query_results(query_results, pos);
                    cout << "--------------------------" << endl;
                    total_tests++;
                }
            }

            // Show
            //viewer.showFrame(colorFrame);
            //QCoreApplication::processEvents();
        }

        // Close Instances
        for (shared_ptr<StreamInstance> instance : instances) {
            instance->close();
        }
    }

    if (num_tests) {
        *num_tests = total_tests;
    }

    return results;
}

QList<DescriptorPtr> PersonReid::create_gallery_DAI4REID_Parsed()
{
    Dataset* dataset = new DAI4REID_Parsed;
    dataset->setPath("C:/datasets/DAI4REID/parse_subset");
    const DatasetMetadata& metadata = dataset->getMetadata();

    QList<DescriptorPtr> gallery;

    // Centroids: Average actor or KMeans, No make up joints, No ignore
    /*QList<QPair<int, int>> centroids = {
        QPair<int, int>(1, 302),
        QPair<int, int>(2, 663),
        QPair<int, int>(3, 746),
        QPair<int, int>(4, 148),
        QPair<int, int>(5, 359)
    };*/

    // Centroids for Occupancy Patterns
    QList<QPair<int, int>> centroids = {
        QPair<int, int>(1, 291),//389
        QPair<int, int>(2, 670),//670
        QPair<int, int>(3, 1162),//1183
        QPair<int, int>(4, 143),//426
        QPair<int, int>(5, 313)//313
    };

    // Centroids: Average actor or Kmeans, Make up joints, No ignore
    /*QList<QPair<int, int>> centroids = {
        QPair<int, int>(1, 302),
        QPair<int, int>(2, 663),
        QPair<int, int>(3, 744),
        QPair<int, int>(4, 157),
        QPair<int, int>(5, 364)
    };*/

    // Create container for read frames
    QHashDataFrames readFrames;

    for (auto centroid : centroids)
    {
        QList<QString> labels;
        shared_ptr<InstanceInfo> instance_info = metadata.instance(centroid.first, 1, centroid.second, labels);

        std::string fileName = instance_info->getFileName(DataFrame::Color).toStdString();

        printf("actor %i sample %i file %s\n", instance_info->getActor(), instance_info->getSample(),
                                               fileName.c_str());

        std::fflush(stdout);

        // Get Sample
        shared_ptr<StreamInstance> instance = dataset->getInstance(*instance_info, DataFrame::Color);

        // Open Instances
        instance->open();

        // Read frames
        instance->readNextFrame(readFrames);

        // Get Frames
        auto colorFrame = static_pointer_cast<ColorFrame>(readFrames.value(DataFrame::Color));
        auto depthFrame = static_pointer_cast<DepthFrame>(readFrames.value(DataFrame::Depth));
        auto maskFrame = static_pointer_cast<MaskFrame>(readFrames.value(DataFrame::Mask));
        auto skeletonFrame = static_pointer_cast<SkeletonFrame>(readFrames.value(DataFrame::Skeleton));
        colorFrame->setOffset(depthFrame->offset());

        // Process
        QList<int> users = skeletonFrame->getAllUsersId();
        shared_ptr<Skeleton> skeleton = skeletonFrame->getSkeleton(users.at(0));
        //DescriptorPtr feature = feature_joints_hist(*colorFrame, *depthFrame, *maskFrame, *skeleton, *instance_info);
        DescriptorPtr feature = feature_skeleton_distances(*colorFrame, *skeleton, *instance_info);

        if (feature) {
            gallery << feature;
        }

        // Close Instances
        instance->close();
    }

    return gallery;
}

void PersonReid::show_images(shared_ptr<ColorFrame> colorFrame, shared_ptr<MaskFrame> maskFrame, shared_ptr<DepthFrame> depthFrame, shared_ptr<Skeleton> skeleton)
{
    // Make Frame
    /*cv::Mat mask_mat(maskFrame->height(), maskFrame->width(), CV_8UC1,
                     (void*) maskFrame->getDataPtr(), maskFrame->getStride());

    filterMask(mask_mat, mask_mat, [&](uchar in, uchar &out){
        out = in == 1 ? 255 : 0;
    });
*/
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
    /*ColorFramePtr skeleton_color = make_shared<ColorFrame>(colorFrame->width(), colorFrame->height());
    skeleton_color->setOffset(colorFrame->offset());
    drawJoints(*(skeleton_color.get()), skeleton->joints());
    cv::Mat skeleton_mat(skeleton_color->height(), skeleton_color->width(), CV_8UC3,
                         (void*) skeleton_color->getDataPtr(), skeleton_color->getStride());*/

    // Show frames
    cv::imshow("Original", color_src);
    //cv::imshow("Mask", mask_mat);
    //cv::imshow("Depth", depth_mat);
    //cv::imshow("Skeleton", skeleton_mat);
    cv::waitKey(1);

    //QCoreApplication::processEvents();

    /*if (instance_info->getSample() == 440) {
        cv::imwrite("paco_src.png", color_src);
        cv::imwrite("paco_mask.png", mask_mat);
        cv::imwrite("paco_skeleton.png", skeleton_mat);
        cv::imwrite("paco_voronoi.png", color_mat);
        cv::waitKey(3000);
    }*/
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
    QList<DescriptorPtr> gallery = train_CAVIAR4REID(actors);

    // Validation
    int num_tests = 0;
    QVector<float> results = validate_CAVIAR4REID(actors, gallery, &num_tests);

    // Show Results
    normalise_results(results, num_tests);
    print_results(results);
}

QList<DescriptorPtr> PersonReid::train_CAVIAR4REID(QList<int> actors)
{
    Dataset* dataset = new CAVIAR4REID;
    dataset->setPath("C:/datasets/CAVIAR4REID");
    const DatasetMetadata& metadata = dataset->getMetadata();

    InstanceViewerWindow viewer;
    viewer.show();

    QList<DescriptorPtr> samples;
    QList<DescriptorPtr> actor_samples;
    QList<DescriptorPtr> centroids;

    // For each actor, compute the feature that minimises the distance to each other sample of
    // the same actor.
    for (int actor : actors)
    {
        QList<shared_ptr<InstanceInfo>> instances = metadata.instances({actor},
                                                                       {1},
                                                                       DatasetMetadata::ANY_LABEL);
        actor_samples.clear();

        for (shared_ptr<InstanceInfo> instance_info : instances)
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
            DescriptorPtr feature = feature_2parts_hist(colorFrame, *instance_info);
            samples << feature;
            actor_samples << feature;

            // Output
            viewer.showFrame(colorFrame);

            // Process Events and sleep
            std::fflush(stdout);
            QCoreApplication::processEvents();
            //QThread::msleep(800);
        }

        DescriptorPtr selectedFeature = Descriptor::minFeature(actor_samples);
        centroids << selectedFeature;
        printf("Init. Centroid %i = actor %i %i\n", actor, selectedFeature->label().getActor(),
                                                       selectedFeature->label().getSample());
    }

    // Learn A: Learning a signature is the same as clustering the input data into num_actor sets
    // and use the centroid of each cluster as model.
    auto kmeans = KMeans<Descriptor>::execute(samples, actors.size(), centroids);
    printClusters(kmeans->getClusters());

    QList<DescriptorPtr> gallery;

    for (auto cluster : kmeans->getClusters()) {
        gallery << cluster.centroid;
    }

    // Learn B: Use the signature that minimises the distance to the rest of signatures
    // of each actor.
    //QList<shared_ptr<Feature>> gallery = centroids;

    return gallery;
}

QVector<float> PersonReid::validate_CAVIAR4REID(const QList<int>& actors, const QList<DescriptorPtr > &gallery, int *num_tests)
{
    Dataset* dataset = new CAVIAR4REID;
    dataset->setPath("C:/datasets/CAVIAR4REID");
    const DatasetMetadata& metadata = dataset->getMetadata();

    QList<shared_ptr<InstanceInfo>> instances = metadata.instances(actors,
                                                                   {2},
                                                                   DatasetMetadata::ANY_LABEL);

    if (num_tests) {
        *num_tests = instances.size();
    }

    QVector<float> results(actors.size());

    for (shared_ptr<InstanceInfo> instance_info : instances)
    {
        // Get Sample
        shared_ptr<StreamInstance> instance = dataset->getInstance(*instance_info, DataFrame::Color);
        instance->open();
        QHashDataFrames readFrames;
        instance->readNextFrame(readFrames);
        shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(readFrames.values().at(0));

        // Feature Extraction
        DescriptorPtr query = feature_2parts_hist(colorFrame, *instance_info);

        // CMC
        QMap<float, int> query_results = compute_distances_to_all_samples(*query, gallery);
        int pos = cummulative_match_curve(query_results, results, query->label().getActor());
        cout << "Results for actor " << instance_info->getActor() << " " << instance_info->getSample() << endl;
        print_query_results(query_results, pos);
        cout << "--------------------------" << endl;
    }

    return results;
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
    DescriptorPtr joint_desc = feature_joint_descriptor(colorFrame, skeleton, instance_info);
    DescriptorPtr region_desc = feature_region_descriptor(colorFrame, depthFrame, maskFrame, skeleton, instance_info);

    //feature->addDescriptor(joints_hist);
    feature->addDescriptor(skeleton_distances);
    feature->addDescriptor(joint_desc);
    feature->addDescriptor(region_desc);

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
    // Build Voronoi cells as a mask
    Skeleton skeleton_tmp = skeleton; // copy
    //makeUpJoints(skeleton_tmp); // Better voronoi
    shared_ptr<MaskFrame> voronoiMask = getVoronoiCells(depthFrame, maskFrame, skeleton_tmp);

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
    QSet<SkeletonJoint::JointType> ignore_joints = {/*SkeletonJoint::JOINT_HEAD,
                                                    SkeletonJoint::JOINT_LEFT_HAND,
                                                    SkeletonJoint::JOINT_RIGHT_HAND*/};

    for (SkeletonJoint& joint : skeleton.joints()) // I use the skeleton of 15 or 20 joints not the temp one.
    {
        if (!ignore_joints.contains(joint.getType())) {
            uchar mask_filter = joint.getType() + 1;
            auto hist = Histogram1c::create(indexed_mat, {0, 255}, voronoi_mat, mask_filter);
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
    shared_ptr<MaskFrame> voronoiMask = getVoronoiCellsParallel(depthFrame, maskFrame, skeleton_tmp);

    cv::Mat voronoi_mat(voronoiMask->height(), voronoiMask->width(), CV_8UC1,
                      (void*) voronoiMask->getDataPtr(), voronoiMask->getStride());

    // Convert image to gray for point detector
    cv::Mat gray_mat;
    cv::cvtColor(color_mat, gray_mat, CV_RGB2GRAY);
    int minHessian = 400;

    shared_ptr<RegionDescriptor> feature = make_shared<RegionDescriptor>(instance_info, colorFrame.getIndex());
    /*QSet<SkeletonJoint::JointType> ignore_joints = {SkeletonJoint::JOINT_HEAD,
                                                    SkeletonJoint::JOINT_LEFT_HAND,
                                                    SkeletonJoint::JOINT_RIGHT_HAND};*/

    cv::SurfFeatureDetector detector;//( minHessian );
    cv::SurfDescriptorExtractor extractor;

    for (SkeletonJoint& joint : skeleton.joints()) // I use the skeleton of 15 or 20 joints not the temp one.
    {
        //if (!ignore_joints.contains(joint.getType()))
        //{
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

            // Add to the feature
            feature->addDescriptor(descriptor);

            /*cv::Mat img_keypoints;
            cv::drawKeypoints(gray_mat, keypoints, img_keypoints, cv::Scalar::all(-1), cv::DrawMatchesFlags::DEFAULT );
            cv::imshow("points", img_keypoints);
            cv::waitKey(1);*/
        //}
    }

    //colorImageWithVoronoid(colorFrame, *voronoiMask);

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
DescriptorPtr PersonReid::feature_joint_descriptor(ColorFrame &colorFrame, Skeleton &skeleton, const InstanceInfo& instance_info) const
{
    cv::Mat color_mat(colorFrame.height(), colorFrame.width(), CV_8UC3,
                      (void*) colorFrame.getDataPtr(), colorFrame.getStride());

    // Make up joints
    Skeleton skeleton_tmp = skeleton; // copy
    makeUpJoints(skeleton_tmp, false);

    // Convert image to gray for point detector
    cv::Mat gray_mat;
    cv::cvtColor(color_mat, gray_mat, CV_RGB2GRAY);

    QSet<SkeletonJoint::JointType> ignore_joints = {//SkeletonJoint::JOINT_HEAD,
                                                    SkeletonJoint::JOINT_CENTER_SHOULDER,
                                                    //SkeletonJoint::JOINT_LEFT_SHOULDER,
                                                    //SkeletonJoint::JOINT_RIGHT_SHOULDER,
                                                    //SkeletonJoint::JOINT_LEFT_ELBOW,
                                                    //SkeletonJoint::JOINT_RIGHT_ELBOW,
                                                    SkeletonJoint::JOINT_LEFT_HAND,
                                                    SkeletonJoint::JOINT_RIGHT_HAND,
                                                    //SkeletonJoint::JOINT_SPINE,
                                                    //SkeletonJoint::JOINT_LEFT_HIP,
                                                    //SkeletonJoint::JOINT_RIGHT_HIP,
                                                    //SkeletonJoint::JOINT_LEFT_KNEE,
                                                    //SkeletonJoint::JOINT_RIGHT_KNEE,
                                                    SkeletonJoint::JOINT_LEFT_FOOT,
                                                    SkeletonJoint::JOINT_RIGHT_FOOT};
    vector<cv::Point2f> points;

    // Get Key Points from the Skeleton Joints
    for (SkeletonJoint& joint : skeleton_tmp.joints())
    {
        if (!ignore_joints.contains(joint.getType()))
        {
            const Point3f& point_3d = joint.getPosition();
            cv::Point2f point_2d;

            Skeleton::convertJointCoordinatesToDepth(point_3d[0], point_3d[1], point_3d[2], &point_2d.x, &point_2d.y);
            point_2d.x = point_2d.x - colorFrame.offset()[0];
            point_2d.y = point_2d.y - colorFrame.offset()[1];

            // Fix points if they are out of range
            if (point_2d.x < 0) point_2d.x = 0;
            else if (point_2d.x >= color_mat.cols) point_2d.x = color_mat.cols - 1;
            if (point_2d.y < 0) point_2d.y = 0;
            else if (point_2d.y >= color_mat.rows) point_2d.y = color_mat.rows - 1;

            //qDebug() << joint.getType() << point_2d.x << point_2d.y << color_mat.cols << color_mat.rows;
            points.push_back(point_2d);
        }
    }

    // Convert points to keypoints
    vector<cv::KeyPoint> key_points;
    cv::KeyPoint::convert(points, key_points, 16);
    for (cv::KeyPoint& kp : key_points) {
        kp.angle = 270.0f;
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
          Point3f::euclideanDistance(knee_left.getPosition(), hip_left.getPosition()));
    */

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
shared_ptr<MaskFrame> PersonReid::getVoronoiCells(const DepthFrame& depthFrame, const MaskFrame& maskFrame, const Skeleton& skeleton) const
{
    Q_ASSERT(depthFrame.width() == maskFrame.width() && depthFrame.height() == maskFrame.height());

    //QElapsedTimer timer;
    //timer.start();

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
                Skeleton::convertDepthCoordinatesToJoint(j+depthFrame.offset()[0], i+depthFrame.offset()[1],
                        depth[j], &point[0], &point[1]);

                /*m_device->convertDepthCoordinatesToJoint(j+depthFrame.offset()[0], i+depthFrame.offset()[1],
                        depth[j], &point[0], &point[1]);*/

                SkeletonJoint closerJoint = getCloserJoint(point, joints);
                mask[j] = closerJoint.getType()+1;
            }
        }
    }

    //qDebug() << "Voronoi Cells" << timer.elapsed();

    return result;
}

shared_ptr<MaskFrame> PersonReid::getVoronoiCellsParallel(const DepthFrame& depthFrame, const MaskFrame& maskFrame, const Skeleton& skeleton) const
{
    Q_ASSERT(depthFrame.width() == maskFrame.width() && depthFrame.height() == maskFrame.height());

    //QElapsedTimer timer;
    //timer.start();

    shared_ptr<MaskFrame> output_mask = static_pointer_cast<MaskFrame>(maskFrame.clone());
    QList<SkeletonJoint> joints = skeleton.joints();

    auto code = [this, depthFrame, joints, &output_mask](int row) -> void
    {
        uint16_t* depth = depthFrame.getRowPtr(row);
        uint8_t* mask = output_mask->getRowPtr(row);

        for (int j=0; j<depthFrame.width(); ++j)
        {
            if (mask[j] > 0)
            {
                Point3f point(0.0f, 0.0f, float(depth[j]));
                Skeleton::convertDepthCoordinatesToJoint(j+depthFrame.offset()[0], row+depthFrame.offset()[1],
                        depth[j], &point[0], &point[1]);

                /*m_device->convertDepthCoordinatesToJoint(j+depthFrame.offset()[0], row+depthFrame.offset()[1],
                            depth[j], &point[0], &point[1]);*/

                SkeletonJoint closerJoint = this->getCloserJoint(point, joints);
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

void PersonReid::drawJoints(ColorFrame& colorFrame, const QList<SkeletonJoint>& joints) {
    // Draw Joints
    for (SkeletonJoint joint : joints) {
        float x, y;
        Skeleton::convertJointCoordinatesToDepth(joint.getPosition()[0],
                joint.getPosition()[1], joint.getPosition()[2], &x, &y);
        /*m_device->convertJointCoordinatesToDepth(joint.getPosition()[0],
                joint.getPosition()[1], joint.getPosition()[2], &x, &y);*/

        if (joint.getType() > SkeletonJoint::JOINT_USER_RESERVED)
            drawPoint(colorFrame, x - colorFrame.offset()[0], y - colorFrame.offset()[1], {255, 0, 0});
        else
            drawPoint(colorFrame, x - colorFrame.offset()[0], y - colorFrame.offset()[1], {0, 0, 255});
    }
}

SkeletonJoint PersonReid::getCloserJoint(const Point3f& cloudPoint, const QList<SkeletonJoint>& joints) const
{
    SkeletonJoint minJoint;
    float minDistance = numeric_limits<float>::max();

    for (SkeletonJoint joint : joints)
    {
        float distance = Point3f::euclideanDistanceSquared(cloudPoint, joint.getPosition());

        if (distance < minDistance) {
            minDistance = distance;
            minJoint = joint; // copy
        }
    }

    return minJoint;
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

void PersonReid::printClusters(const QList<Cluster<Descriptor> > &clusters) const
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
}

void PersonReid::makeUpJoints(Skeleton& skeleton, bool only_middle_points) const
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

template <class T, int N>
void printHistogram(const Histogram<T, N> &hist, int n_elems)
{
    // Show info of the histogram
    int i=0;
    foreach (auto item, hist.higherFreqBins(n_elems)) {
        qDebug() << i++ << "(" << item->point.toString() << ")" << item->value << item->dist;
    }
}


// Approach 1: log color space (2 channels) without Histogram!!
// Paper: Color Invariants for Person Reidentification
void PersonReid::approach1(QHashDataFrames &frames)
{
    Q_ASSERT(frames.contains(DataFrame::Color) && frames.contains(DataFrame::Mask) &&
             frames.contains(DataFrame::Skeleton) && frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(frames.value(DataFrame::Metadata));

    // Work in the Bounding Box
    if (metadataFrame->boundingBoxes().isEmpty())
        return;

    BoundingBox bb = metadataFrame->boundingBoxes().first();
    shared_ptr<ColorFrame> subColorFrame = colorFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> subMaskFrame = maskFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                             bb.size().width(), bb.size().height());

    // Use cv::Mat for my color frame and mask frame
    cv::Mat inputImg(subColorFrame->height(), subColorFrame->width(),
                 CV_8UC3, (void*)subColorFrame->getDataPtr(), subColorFrame->getStride());

    cv::Mat mask(subMaskFrame->height(), subMaskFrame->width(),
             CV_8UC1, (void*)subMaskFrame->getDataPtr(), subMaskFrame->getStride());

    // Compute Upper and Lower Masks
    cv::Mat upper_mask, lower_mask;
    computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask, mask);

    // Sample points
    const int num_samples = 85;
    QList<Point3b> u_list = randomSampling<uchar, 3>(inputImg, num_samples, upper_mask);
    QList<Point3b> l_list = randomSampling<uchar, 3>(inputImg, num_samples, lower_mask);

    // Convert from RGB color space to 2D log-chromacity color space
    QList<Point2f> log_u_list = convertRGB2Log2DAsList(u_list);
    QList<Point2f> log_l_list = convertRGB2Log2DAsList(l_list);

    // Create a 2D Image of the color space
    float log_range[] = {-5.6f, 5.6f};
    cv::Mat logCoordImg;
    create2DCoordImage({&log_u_list, &log_l_list},
                       {cv::Vec3b(0, 0, 255), cv::Vec3b(255, 0, 0)}, // Red (upper), Blue (lower)
                       logCoordImg,
                       log_range);

    // Show
    cv::imshow("Log.Img", logCoordImg);
    cv::waitKey(1);
}

// Approach 2: CIElab (2 channels)
// Paper: Color Invariants for Person Reidentification
void PersonReid::approach2(QHashDataFrames &frames)
{
    Q_ASSERT(frames.contains(DataFrame::Color) && frames.contains(DataFrame::Mask) &&
             frames.contains(DataFrame::Skeleton) && frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(frames.value(DataFrame::Metadata));

    // Work in the Bounding Box
    if (metadataFrame->boundingBoxes().isEmpty())
        return;

    BoundingBox bb = metadataFrame->boundingBoxes().first();
    shared_ptr<ColorFrame> subColorFrame = colorFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> subMaskFrame = maskFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                             bb.size().width(), bb.size().height());

    // Start OpenCV code
    {using namespace cv;

        // Use cv::Mat for my color frame and mask frame
        Mat inputImg(subColorFrame->height(), subColorFrame->width(),
                     CV_8UC3, (void*)subColorFrame->getDataPtr(), subColorFrame->getStride());

        Mat mask(subMaskFrame->height(), subMaskFrame->width(),
                 CV_8UC1, (void*)subMaskFrame->getDataPtr(), subMaskFrame->getStride());

        // Compute Upper and Lower Masks
        Mat upper_mask, lower_mask;
        computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask, mask); // This ones modifies inputImg

        // Sample Mask
        Mat upper_sampled_mask = upper_mask; //randomSampling<uchar>(upper_mask, 200, upper_mask);
        Mat lower_sampled_mask = lower_mask; //randomSampling<uchar>(lower_mask, 200, lower_mask);

        // Denoise Image
        //denoiseImage(copyImg, copyImg);

        // Convert from RGB color space to CIElab
        // CIElab 8 bits image: all channels range is 0-255.
        Mat imgLab;
        cv::cvtColor(inputImg, imgLab, cv::COLOR_RGB2Lab);

        // Split image in L*, a* and b* channels
        vector<Mat> lab_planes;
        split(imgLab, lab_planes);

        // Create 2D image with only a and b channels
        vector<Mat> ab_planes = {lab_planes[1], lab_planes[2]};
        Mat imgAb;
        merge(ab_planes, imgAb);

        // Compute the histogram for the upper (torso) and lower (leggs) parts
        auto u_hist = Histogram2D<uchar>::create(imgAb, {0, 255}, upper_sampled_mask);
        auto l_hist = Histogram2D<uchar>::create(imgAb, {0, 255}, lower_sampled_mask);

        const int n_often_colors = 8;

        // Create an image of the distribution of the histogram
        Mat histDist;
        createHistImage<uchar,2>({u_hist.get(), l_hist.get()},
                                     {Scalar(255, 0, 0), Scalar(0, 0, 255)}, // Blue (upper), Red (lower)
                                     histDist);

        // Create a 2D Image of the histogram
        float ab_range[] = {0.0f, 255.0f};
        Mat histImg;
        create2DCoordImage<uchar>({u_hist.get(), l_hist.get()},
                                  n_often_colors,
                                  {Vec3b(255, 0, 0), Vec3b(0, 0, 255)}, // Blue (upper), Red (lower)
                                  histImg,
                                  ab_range);

        // Modify input image to show used mask
        colorImageWithMask(inputImg, inputImg, upper_mask, lower_mask);

        // Show
        imshow("Hist.Dist", histDist);
        imshow("Hist.Img", histImg);
        waitKey(1);

    } // End OpenCV code

    return;
}

// Approach 3: YUV (2 channels)
// Paper: Color Invariants for Person Reidentification
void PersonReid::approach3(QHashDataFrames& frames)
{
    Q_ASSERT(frames.contains(DataFrame::Color) && frames.contains(DataFrame::Mask) &&
             frames.contains(DataFrame::Skeleton) && frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(frames.value(DataFrame::Metadata));

    // Work in the Bounding Box
    if (metadataFrame->boundingBoxes().isEmpty())
        return;

    BoundingBox bb = metadataFrame->boundingBoxes().first();
    shared_ptr<ColorFrame> subColorFrame = colorFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> subMaskFrame = maskFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                             bb.size().width(), bb.size().height());

    // Start OpenCV code
    {using namespace cv;

        // Use cv::Mat for my color frame and mask frame
        Mat inputImg(subColorFrame->height(), subColorFrame->width(),
                     CV_8UC3, (void*)subColorFrame->getDataPtr(), subColorFrame->getStride());

        Mat mask(subMaskFrame->height(), subMaskFrame->width(),
                 CV_8UC1, (void*)subMaskFrame->getDataPtr(), subMaskFrame->getStride());


        // Compute Upper and Lower Masks
        Mat upper_mask, lower_mask;
        computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask, mask); // This ones modifies inputImg

        // Sample Mask
        Mat upper_sampled_mask = upper_mask; //randomSampling<uchar>(upper_mask, 200, upper_mask);
        Mat lower_sampled_mask = lower_mask; //randomSampling<uchar>(lower_mask, 200, lower_mask);

        // Denoise Image
        //denoiseImage(copyImg, copyImg);

        // Convert from RGB color space to YCrCb
        Mat imgYuv;
        cv::cvtColor(inputImg, imgYuv, cv::COLOR_RGB2YCrCb);

        // Split image in Y, u and v channels
        vector<Mat> yuv_planes;
        split(imgYuv, yuv_planes);

        // Create 2D image with only u and v channels
        vector<Mat> uv_planes = {yuv_planes[1], yuv_planes[2]};
        Mat imgUv;
        merge(uv_planes, imgUv);

        // Compute the histogram for the upper (torso) and lower (leggs) parts
        auto u_hist = Histogram2D<uchar>::create(imgUv, {0, 255}, upper_sampled_mask);
        auto l_hist = Histogram2D<uchar>::create(imgUv, {0, 255}, lower_sampled_mask);

        const int n_often_colors = 8;

        // Show info of the histogram
        printHistogram<uchar, 2>(*u_hist, n_often_colors);

        // Create an image of the distribution of the histogram
        Mat histDist;
        createHistImage<uchar,2>({u_hist.get(), l_hist.get()},
                                     {Scalar(255, 0, 0), Scalar(0, 0, 255)}, // Blue (upper), Red (lower)
                                     histDist);

        // Create a 2D Image of the histogram
        float uv_range[] = {0.0f, 255.0f};
        Mat histImg;
        create2DCoordImage<uchar>({u_hist.get(), l_hist.get()},
                                  n_often_colors,
                                  {Vec3b(255, 0, 0), Vec3b(0, 0, 255)}, // Blue (upper), Red (lower)
                                  histImg,
                                  uv_range);

        // Modify input image to show used mask
        colorImageWithMask(inputImg, inputImg, upper_mask, lower_mask);

        // Show
        imshow("Hist.Dist", histDist);
        imshow("Hist.Img", histImg);
        waitKey(1);

    } // End OpenCV code

    return;
}

// Approach 4: RGB
// Paper: Color Invariants for Person Reidentification
void PersonReid::approach4(QHashDataFrames &frames)
{
    Q_ASSERT(frames.contains(DataFrame::Color) && frames.contains(DataFrame::Mask) &&
             frames.contains(DataFrame::Skeleton) && frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(frames.value(DataFrame::Metadata));

    // Work in the Bounding Box
    if (metadataFrame->boundingBoxes().isEmpty())
        return;

    BoundingBox bb = metadataFrame->boundingBoxes().first();
    shared_ptr<ColorFrame> roiColorFrame = colorFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> roiMaskFrame = maskFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                             bb.size().width(), bb.size().height());

    // Start OpenCV code
    {using namespace cv;

        // Use cv::Mat for my color frame and mask frame
        Mat inputImg(roiColorFrame->height(), roiColorFrame->width(),
                     CV_8UC3, (void*)roiColorFrame->getDataPtr(), roiColorFrame->getStride());

        Mat mask(roiMaskFrame->height(), roiMaskFrame->width(),
                 CV_8UC1, (void*)roiMaskFrame->getDataPtr(), roiMaskFrame->getStride());

        // Denoise Image
        denoiseImage(inputImg, inputImg);

        // Discretise Image
        //discretiseRGBImage(inputImg, inputImg); // TEST: in YUV Space not RGB

        // Compute Upper and Lower Masks
        Mat upper_mask, lower_mask;
        computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask, mask);

        // Sample Mask
        Mat upper_sampled_mask = upper_mask; //randomSampling<uchar>(upper_mask, 1500, upper_mask);
        Mat lower_sampled_mask = lower_mask; //randomSampling<uchar>(lower_mask, 1500, lower_mask);

        // Convert to another color space
        Mat imgYuv;
        cv::cvtColor(inputImg, imgYuv, cv::COLOR_RGB2YCrCb); // YUV

        // Compute the histogram for the upper (torso) and lower (leggs) parts
        auto u_hist = Histogram3D<uchar>::create(imgYuv, {0, 255}, upper_sampled_mask);
        auto l_hist = Histogram3D<uchar>::create(imgYuv, {0, 255}, lower_sampled_mask);

        const int n_often_items = 72;

        // Create Distribution
        Mat distImg;
        createHistImage<uchar,3>({u_hist.get(), l_hist.get()},
                                     {Scalar(0, 255, 255), Scalar(0, 0, 255)}, // Blue (upper hist), Red (lower hist)
                                     distImg);

        // Create it on an image
        Mat colorPalette;
        create2DColorPalette<uchar>(u_hist->higherFreqBins(n_often_items), l_hist->higherFreqBins(n_often_items), colorPalette);
        cv::cvtColor(colorPalette, colorPalette, cv::COLOR_YCrCb2BGR); // YUV to BGR

        // Show
        imshow("Dist.Hist", distImg);
        imshow("Palette", colorPalette);
        waitKey(1);

    } // End OpenCV code
}

// Approach 5: RGB with buffering
// Paper: Color Invariants for Person Reidentification
void PersonReid::approach5(QHashDataFrames& frames)
{
    Q_ASSERT(frames.contains(DataFrame::Color) && frames.contains(DataFrame::Mask) &&
             frames.contains(DataFrame::Skeleton) && frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(frames.value(DataFrame::Metadata));

    // Work in the Bounding Box
    if (metadataFrame->boundingBoxes().isEmpty())
        return;

    BoundingBox bb = metadataFrame->boundingBoxes().first();
    shared_ptr<ColorFrame> roiColorFrame = colorFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> roiMaskFrame = maskFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                             bb.size().width(), bb.size().height());

    static int frame_counter = 0;
    static const int buffer_size = 5;
    const int n_often_items = 72;
    static cv::Mat color_frame_vector[buffer_size];
    static cv::Mat mask_frame_vector[buffer_size];
    static bool finished = false;

    // Start OpenCV code
    {using namespace cv;

        // Use cv::Mat for my color frame and mask frame
        Mat inputImg(roiColorFrame->height(), roiColorFrame->width(),
                     CV_8UC3, (void*)roiColorFrame->getDataPtr(), roiColorFrame->getStride());

        Mat mask(roiMaskFrame->height(), roiMaskFrame->width(),
                 CV_8UC1, (void*)roiMaskFrame->getDataPtr(), roiMaskFrame->getStride());

        //
        // Image Pre-Processing
        //
        Mat img = inputImg.clone();

        // Denoise Image
        denoiseImage(img, img);

        // Discretise Image
        //discretiseRGBImage(img, img); // TEST: in YUV Space not RGB

        // Convert to another color space
        cv::cvtColor(img, img, cv::COLOR_RGB2YCrCb); // YUV

        //
        // Buffering
        //
        int current_idx = frame_counter % buffer_size;
        color_frame_vector[current_idx] = img;
        mask_frame_vector[current_idx] = mask.clone();

        // Start to do things when I fill up the buffer
        if (frame_counter >= buffer_size-1)
        {
            // Compute accumulated histogram of the current frame plus the previous fourth
            Histogram3c u_hist_acc, l_hist_acc;

            for (int i=0; i<buffer_size; ++i)
            {
                // Compute Upper and Lower Masks
                Mat upper_mask, lower_mask;
                computeUpperAndLowerMasks(color_frame_vector[i], upper_mask, lower_mask, mask_frame_vector[i]);

                // Sample Mask
                Mat upper_sampled_mask = upper_mask; //randomSampling<uchar>(upper_mask, 1500, upper_mask);
                Mat lower_sampled_mask = lower_mask; //randomSampling<uchar>(lower_mask, 1500, lower_mask);


                // Compute the histogram for the upper (torso) and lower (leggs) parts of each frame in the buffer
                auto u_hist_t = Histogram3D<uchar>::create(color_frame_vector[i], {0, 255}, upper_sampled_mask);
                auto l_hist_t = Histogram3D<uchar>::create(color_frame_vector[i], {0, 255}, lower_sampled_mask);

                // Accumulate it
                u_hist_acc += *u_hist_t;
                l_hist_acc += *l_hist_t;
            }

            // Show it on an image
            Mat colorPalette_acc;
            create2DColorPalette<uchar>(u_hist_acc.higherFreqBins(n_often_items), l_hist_acc.higherFreqBins(n_often_items), colorPalette_acc);
            cv::cvtColor(colorPalette_acc, colorPalette_acc, cv::COLOR_YCrCb2BGR); // YUV to BGR

            // Show Distribution
            Mat distImg;
            createHistImage<uchar,3>({&u_hist_acc, &l_hist_acc},
                                         {Scalar(255, 0, 0), Scalar(0, 0, 255)}, // Blue (upper hist), Red (lower hist)
                                         distImg);

            // Show
            imshow("Palette AC", colorPalette_acc);
            imshow("Hist.dist", distImg);
            waitKey(1);
        }
        else {
            qDebug() << "Buffering " << frame_counter % buffer_size;
        }

    } // End OpenCV code

    frame_counter++;
}

// Approach 6: Indexed colors
void PersonReid::approach6(QHashDataFrames &frames)
{
    Q_ASSERT(frames.contains(DataFrame::Color) && frames.contains(DataFrame::Mask) &&
             frames.contains(DataFrame::Skeleton) && frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(frames.value(DataFrame::Metadata));

    // Work in the Bounding Box
    if (metadataFrame->boundingBoxes().isEmpty())
        return;

    BoundingBox bb = metadataFrame->boundingBoxes().first();
    shared_ptr<ColorFrame> roiColorFrame = colorFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> roiMaskFrame = maskFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                             bb.size().width(), bb.size().height());

    static int frame_counter = 0;
    static const int buffer_size = 5; // 190
    static cv::Mat color_frame_vector[buffer_size];
    static cv::Mat mask_frame_vector[buffer_size];

    // Start OpenCV code
    {using namespace cv;

        // Use cv::Mat for my color frame and mask frame
        Mat inputImg(roiColorFrame->height(), roiColorFrame->width(),
                     CV_8UC3, (void*)roiColorFrame->getDataPtr(), roiColorFrame->getStride());

        Mat mask(roiMaskFrame->height(), roiMaskFrame->width(),
                 CV_8UC1, (void*)roiMaskFrame->getDataPtr(), roiMaskFrame->getStride());

        // Color Palette with 8-8-4 levels
        Mat indexedImg = convertRGB2Indexed884(inputImg);

        //
        // Buffering
        //
        int current_idx = frame_counter % buffer_size;
        color_frame_vector[current_idx] = indexedImg;
        mask_frame_vector[current_idx] = mask.clone();

        // Start to do things when I fill up the buffer
        if (frame_counter >= buffer_size-1)
        {
            QList<shared_ptr<Histogram1c>> samples;
            Histogram1c u_hist_acc, l_hist_acc;

            for (int i=0; i<buffer_size; ++i)
            {
                // Compute Upper and Lower Masks
                Mat upper_mask, lower_mask;
                computeUpperAndLowerMasks(color_frame_vector[i], upper_mask, lower_mask, mask_frame_vector[i]);

                // Sample Mask
                Mat upper_sampled_mask = upper_mask; //randomSampling<uchar>(upper_mask, 1500, upper_mask);
                Mat lower_sampled_mask = lower_mask; //randomSampling<uchar>(lower_mask, 1500, lower_mask);

                // Compute the histogram for the upper (torso) and lower (leggs) parts of each frame in the buffer
                auto u_hist_t = Histogram1c::create(color_frame_vector[i], {0, 255}, upper_sampled_mask);
                auto l_hist_t = Histogram1c::create(color_frame_vector[i], {0, 255}, lower_sampled_mask);

                // Add to samples list
                samples << u_hist_t << l_hist_t;

                // Accumulate it
                u_hist_acc += *u_hist_t;
                l_hist_acc += *l_hist_t;
            }

            qDebug() << "u.items" << u_hist_acc.numItems() << "l.items" << l_hist_acc.numItems();
            qDebug() << "dist.ul" << Histogram1c::intersection(u_hist_acc, l_hist_acc);

            // KMeans
            qDebug() << "Computing K-Means...";
            const int k = 2;
            auto kmeans = KMeans<Histogram1c>::execute(samples, k);
            QList<Cluster<Histogram1c>> clusters = kmeans->getClusters();

            Mat hist_img[k];
            int i = 0;

            foreach (Cluster<Histogram1c> cluster, clusters)
            {
                if (!cluster.samples.isEmpty()) {
                    qDebug() << "Cluster" << i << "size" << cluster.samples.size();
                    shared_ptr<Histogram1c> hist = cluster.centroid;
                    createHistImage<uchar,1>({hist.get()}, {Scalar(0, 0, 255)}, hist_img[i]);
                    ++i;
                }
            }

            qDebug() << "Job Done";

            /*Mat hist_img[k];
            int img_idx = 0;

            foreach (shared_ptr<DistanceComparable> item, kmeans->getCentroids())
            {
                shared_ptr<Histogram3D<uchar>> hist = static_pointer_cast<Histogram3D<uchar>>(item);
                createHistImage(hist->sortedItems(n_often_items), hist_img[img_idx]);
                cv::cvtColor(hist_img[img_idx], hist_img[img_idx], cv::COLOR_YCrCb2BGR); // YUV to BGR
                img_idx++;
            }

            imshow("centroid1", hist_img[0]);
            imshow("centroid2", hist_img[1]);
            imshow("centroid3", hist_img[2]);
            imshow("centroid4", hist_img[3]);*/

            // Show it on an image
            /*Mat colorPalette_acc;
            create2DColorPalette<uchar>(u_hist_acc.sortedItems(n_often_items), l_hist_acc.sortedItems(n_often_items), colorPalette_acc);
            cv::cvtColor(colorPalette_acc, colorPalette_acc, cv::COLOR_YCrCb2BGR);*/ // YUV to BGR

            // Show Distribution
            /*Mat distImg;
            createHistDistImage<uchar,1>({&u_hist_acc, &l_hist_acc},
                                       0,
                                       {Scalar(0, 255, 255), Scalar(0, 0, 255)}, // Blue (upper hist), Red (lower hist)
                                       distImg);*/
            // Show
            //imshow("Palette AC", colorPalette_acc);

            //imshow("Hist.dist", distImg);
            imshow("hist1", hist_img[0]);
            imshow("hist2", hist_img[1]);
            waitKey(1);
        }
        else {
            qDebug() << "Buffering " << frame_counter % buffer_size;
        }
    } // End OpenCV code

    frame_counter++;
}

} // End Namespace



