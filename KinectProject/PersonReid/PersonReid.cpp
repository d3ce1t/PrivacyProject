#include "PersonReid.h"
#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include "openni/OpenNIColorInstance.h"
#include "openni/OpenNIUserTrackerInstance.h"
#include "dataset/CAVIAR4REID/CAVIAR4REID.h"
#include "dataset/DAI4REID/DAI4REID.h"
#include "dataset/DAI4REID_Parsed/DAI4REID_Parsed.h"
#include "viewer/InstanceViewerWindow.h"
#include "Config.h"
#include "opencv_utils.h"
#include "types/Histogram.h"
#include <QThread>
#include <QImage>
#include <QFile>
#include <future>
#include <QCryptographicHash>


//using namespace std;

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
    //test_DAI4REID();
    //test_CAVIAR4REID();
    test_DAI4REID_Parsed();
    //test2();
    //test3();

    QCoreApplication::instance()->quit();
}

void PersonReid::parseDataset()
{
    Dataset* dataset = new DAI4REID;
    dataset->setPath("C:/datasets/DAI4REID");
    const DatasetMetadata& metadata = dataset->getMetadata();

    // Create memory for colorFrame
    QHashDataFrames readFrames = allocateMemory();

    QList<int> actors = {4,5};//{1, 2, 3};

    // For each actor, compute the feature that minimises the distance to each other sample of
    // the same actor.
    foreach (int actor, actors)
    {
        QList<shared_ptr<InstanceInfo>> instances_md = metadata.instances({actor},
                                                                       {1},
                                                                       DatasetMetadata::ANY_LABEL);

        shared_ptr<InstanceInfo> instance_info = instances_md.at(0);

        std::string fileName = instance_info->getFileName(DataFrame::Color).toStdString();

        printf("actor %i sample %i file %s\n", instance_info->getActor(),
               instance_info->getSample(),
               fileName.c_str());

        std::fflush(stdout);

        // Get Sample
        QList<shared_ptr<StreamInstance>> instances;
        instances << dataset->getInstance(*instance_info, DataFrame::Color);
        instances << dataset->getInstance(*instance_info, DataFrame::Metadata);

        // Open Instances
        foreach (shared_ptr<StreamInstance> instance, instances) {
            instance->open();
        }

        shared_ptr<OpenNIColorInstance> colorInstance = static_pointer_cast<OpenNIColorInstance>(instances.at(0));
        colorInstance->device().playbackControl()->setSpeed(0.10f);

        // Read frames
        int previousFrame = 0;

        while (colorInstance->hasNext())
        {
            foreach (shared_ptr<StreamInstance> instance, instances) {
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

            // Process
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
        foreach (shared_ptr<StreamInstance> instance, instances) {
            instance->close();
        }
    }
}

// Test feature of n-parts localised histograms (corresponding to joints) with OpenNI
void PersonReid::test_DAI4REID()
{
    QList<int> actors = {1, 2, 3, 4, 5};

    // Training
    QList<shared_ptr<Feature>> gallery = train_DAI4REID(actors);

    // Validation
    int num_tests = 0;
    QVector<float> results = validate_DAI4REID(actors, gallery, &num_tests);

    // Show Results
    normalise_results(results, num_tests);
    print_results(results);
}

QList<shared_ptr<Feature>> PersonReid::train_DAI4REID(QList<int> actors)
{
    Dataset* dataset = new DAI4REID;
    dataset->setPath("C:/datasets/DAI4REID");
    const DatasetMetadata& metadata = dataset->getMetadata();

    InstanceViewerWindow viewer;
    viewer.show();

    QList<shared_ptr<Feature>> samples;
    QList<shared_ptr<Feature>> actor_samples;
    QList<shared_ptr<Feature>> centroids;

    // Create memory for colorFrame
    QHashDataFrames readFrames = allocateMemory();

    // For each actor, compute the feature that minimises the distance to each other sample of
    // the same actor.
    foreach (int actor, actors)
    {
        QList<shared_ptr<InstanceInfo>> instances = metadata.instances({actor},
                                                                       {2},
                                                                       DatasetMetadata::ANY_LABEL);

        foreach (shared_ptr<InstanceInfo> instance_info, instances)
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
            foreach (shared_ptr<StreamInstance> instance, instances) {
                instance->open();
            }

            shared_ptr<OpenNIColorInstance> colorInstance = static_pointer_cast<OpenNIColorInstance>(instances.at(0));
            colorInstance->device().playbackControl()->setSpeed(1.0f);
            m_device = &(colorInstance->device());

            // Read frames
            while (colorInstance->hasNext())
            {
                foreach (shared_ptr<StreamInstance> instance, instances) {
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

                    shared_ptr<Feature> feature = feature_joints_hist(*roiColor, *roiDepth, *roiMask, *skeleton,
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
            foreach (shared_ptr<StreamInstance> instance, instances) {
                instance->close();
            }
        }

        qDebug() << "Actor samples" << actor_samples.size();
        shared_ptr<Feature> selectedFeature = Feature::minFeature(actor_samples);
        centroids << selectedFeature;
        printf("Init. Centroid %i = actor %i %i %i\n", actor, selectedFeature->label().getActor(),
                                                       selectedFeature->label().getSample(),
                                                       selectedFeature->frameId());
        actor_samples.clear();
        std::fflush(stdout);
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

QVector<float> PersonReid::validate_DAI4REID(const QList<int> &actors, const QList<shared_ptr<Feature>>& gallery, int *num_tests)
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

    foreach (shared_ptr<InstanceInfo> instance_info, instances)
    {
        // Get Sample
        QList<shared_ptr<StreamInstance>> instances;
        instances << dataset->getInstance(*instance_info, DataFrame::Color);
        //instances << dataset->getInstance(*instance_info, DataFrame::Depth);     // I should FIX this
        //instances << dataset->getInstance(*instance_info, DataFrame::Skeleton);  // because I don't need
        //instances << dataset->getInstance(*instance_info, DataFrame::Mask);      // to know imp. details
        instances << dataset->getInstance(*instance_info, DataFrame::Metadata);

        // Open Instances
        foreach (shared_ptr<StreamInstance> instance, instances) {
            instance->open();
        }

        shared_ptr<OpenNIColorInstance> colorInstance = static_pointer_cast<OpenNIColorInstance>(instances.at(0));
        colorInstance->device().playbackControl()->setSpeed(0.25f);
        m_device = &(colorInstance->device());

        // Read frames
        while (colorInstance->hasNext())
        {
            foreach (shared_ptr<StreamInstance> instance, instances) {
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

                shared_ptr<Feature> query = feature_joints_hist(*roiColor, *roiDepth, *roiMask, *skeleton,
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
        foreach (shared_ptr<StreamInstance> instance, instances) {
            instance->close();
        }
    }

    if (num_tests) {
        *num_tests = total_tests;
    }

    return results;
}

void PersonReid::test_DAI4REID_Parsed()
{
    QList<int> actors = {1, 2, 3, 4, 5};

    // Training
    QList<shared_ptr<Feature>> gallery = train_DAI4REID_Parsed(actors);
    //QList<shared_ptr<Feature>> gallery = create_gallery_DAI4REID_Parsed();

    for (shared_ptr<Feature> target : gallery) {
        qDebug() << target->label().getActor() << target->label().getSample();
    }

    // Validation
    int num_tests = 0;
    QVector<float> results = validate_DAI4REID_Parsed(actors, gallery, &num_tests);

    // Show Results
    normalise_results(results, num_tests);
    print_results(results);
}

QList<shared_ptr<Feature>> PersonReid::create_gallery_DAI4REID_Parsed()
{
    Dataset* dataset = new DAI4REID_Parsed;
    dataset->setPath("C:/datasets/DAI4REID/parse_subset");
    const DatasetMetadata& metadata = dataset->getMetadata();

    QList<shared_ptr<Feature>> gallery;

    // Centroids: Average actor or KMeans, No make up joints, No ignore
    QList<QPair<int, int>> centroids = {
        QPair<int, int>(1, 302),
        QPair<int, int>(2, 663),
        QPair<int, int>(3, 746),
        QPair<int, int>(4, 148),
        QPair<int, int>(5, 359)
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
        shared_ptr<Feature> feature = feature_joints_hist(*colorFrame, *depthFrame, *maskFrame, *skeleton,
                                                          *instance_info);
        if (feature) {
            gallery << feature;
        }

        // Close Instances
        instance->close();
    }

    return gallery;
}

QList<shared_ptr<Feature>> PersonReid::train_DAI4REID_Parsed(QList<int> actors)
{
    Dataset* dataset = new DAI4REID_Parsed;
    dataset->setPath("C:/datasets/DAI4REID/parse_subset");
    const DatasetMetadata& metadata = dataset->getMetadata();

    QList<shared_ptr<Feature>> samples;
    QList<shared_ptr<Feature>> actor_samples;
    QList<shared_ptr<Feature>> centroids;

    // Create container for read frames
    QHashDataFrames readFrames;

    // For each actor, compute the feature that minimises the distance to each other sample of
    // the same actor.
    foreach (int actor, actors)
    {
        QList<shared_ptr<InstanceInfo>> instances = metadata.instances({actor},
                                                                       {1},
                                                                       DatasetMetadata::ANY_LABEL);

        foreach (shared_ptr<InstanceInfo> instance_info, instances)
        {
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
            //highLightMask(*colorFrame, *maskFrame);
            //highLightDepth(*colorFrame, *depthFrame);
            //drawJoints(*colorFrame, skeleton->joints());
            shared_ptr<Feature> feature = feature_joints_hist(*colorFrame, *depthFrame, *maskFrame, *skeleton,
                                                              *instance_info);
            if (feature) {
                actor_samples << feature;
                samples << feature;
            }

            // Show
            /*cv::Mat color_mat(colorFrame->height(), colorFrame->width(), CV_8UC3,
                              (void*) colorFrame->getDataPtr(), colorFrame->getStride());
            cv::imshow("Image", color_mat);
            cv::waitKey(1);*/
            //QCoreApplication::processEvents();

            // Close Instances
            instance->close();
        }

        qDebug() << "Actor samples" << actor_samples.size();
        shared_ptr<Feature> selectedFeature = Feature::minFeatureParallel(actor_samples);
        centroids << selectedFeature;
        printf("Init. Centroid %i = actor %i %i %i\n", actor, selectedFeature->label().getActor(),
                                                       selectedFeature->label().getSample(),
                                                       selectedFeature->frameId());
        actor_samples.clear();
        std::fflush(stdout);
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
    QList<shared_ptr<Feature>> gallery = centroids;

    return gallery;
}

QVector<float> PersonReid::validate_DAI4REID_Parsed(const QList<int> &actors, const QList<shared_ptr<Feature>>& gallery, int *num_tests)
{
    Dataset* dataset = new DAI4REID_Parsed;
    dataset->setPath("C:/datasets/DAI4REID/parse_subset");
    const DatasetMetadata& metadata = dataset->getMetadata();
    int total_tests = 0;

    QList<shared_ptr<InstanceInfo>> instances = metadata.instances(actors,
                                                                   {2},
                                                                   DatasetMetadata::ANY_LABEL);

    // Create container for read frames
    QHashDataFrames readFrames;

    // Start validation
    QVector<float> results(actors.size());

    foreach (shared_ptr<InstanceInfo> instance_info, instances)
    {
        std::string fileName = instance_info->getFileName(DataFrame::Color).toStdString();

        printf("actor %i sample %i file %s\n", instance_info->getActor(),
               instance_info->getSample(),
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

        shared_ptr<Feature> query = feature_joints_hist(*colorFrame, *depthFrame, *maskFrame, *skeleton,
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

        // Close Instances
        instance->close();
    }

    if (num_tests) {
        *num_tests = total_tests;
    }

    return results;
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
        QList<shared_ptr<InstanceInfo>> instances = metadata.instances({actor},
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
            //QThread::msleep(800);
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

    QList<shared_ptr<InstanceInfo>> instances = metadata.instances(actors,
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
    QMap<float, int> query_results; // distance, actor

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

shared_ptr<Feature> PersonReid::feature_joints_hist(ColorFrame& colorFrame, DepthFrame& depthFrame,
                                           MaskFrame&  maskFrame, Skeleton& skeleton, const InstanceInfo &instance_info)
{
    // Build Voronoi cells as a mask
    Skeleton skeleton_tmp = skeleton; // copy
    //makeUpJoints(skeleton_tmp);
    shared_ptr<MaskFrame> voronoiMask = getVoronoiCellsParallel(depthFrame, maskFrame, skeleton_tmp);

    //QElapsedTimer timer;
    //timer.start();
    // Compute histograms for each Voronoi cell obtained from joints
    cv::Mat color_mat(colorFrame.height(), colorFrame.width(), CV_8UC3,
                      (void*) colorFrame.getDataPtr(), colorFrame.getStride());

    cv::Mat voronoi_mat(voronoiMask->height(), voronoiMask->width(), CV_8UC1,
                      (void*) voronoiMask->getDataPtr(), voronoiMask->getStride());

    // Convert image to 4096 colors using a color Palette with 16-16-16 levels
    //cv::Mat indexed_mat = dai::convertRGB2Indexed161616(color_mat);

    // Convert image to 256 colors using a color Palette with 8-8-4 levels
    //cv::Mat indexed_mat = dai::convertRGB2Indexed884(color_mat);

    // Convert image to HSV
    cv::Mat hsv_mat;
    cv::cvtColor(color_mat, hsv_mat, CV_RGB2HSV);

    std::vector<cv::Mat> hsv_planes;
    cv::split(hsv_mat, hsv_planes);

    std::vector<cv::Mat> hs_planes = {hsv_planes[0], hsv_planes[1]};
    cv::Mat hs_mat;
    cv::merge(hs_planes, hs_mat);
    //cv::Mat indexed_mat = hsv_planes[0];
    cv::Mat indexed_mat = hs_mat;

    shared_ptr<Feature> feature = make_shared<Feature>(instance_info, colorFrame.getIndex());
    QSet<SkeletonJoint::JointType> ignore_joints; /* = {SkeletonJoint::JOINT_HEAD,
                                                    SkeletonJoint::JOINT_LEFT_HAND,
                                                    SkeletonJoint::JOINT_RIGHT_HAND};*/

    foreach (SkeletonJoint joint, skeleton.joints()) // I use the skeleton of 15 or 20 joints not the temp one.
    {
        if (!ignore_joints.contains(joint.getType())) {
            // Calculate histogram and store it
            uchar mask_filter = joint.getType() + 1;
            auto hist = Histogram2c::create(indexed_mat, {0, 255}, voronoi_mat, mask_filter);
            feature->addHistogram(*hist);
        }
    }

    //qDebug() << "Histograms" << timer.elapsed();

    //colorImageWithVoronoid(colorFrame, *voronoiMask);

    return feature;
}

shared_ptr<Feature> PersonReid::feature_2parts_hist(shared_ptr<ColorFrame> colorFrame, const InstanceInfo& instance_info) const
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
    auto u_hist = Histogram2c::create(indexedImg, {0, 255}, upper_mask);
    auto l_hist = Histogram2c::create(indexedImg, {0, 255}, lower_mask);

    dai::colorImageWithMask(inputImg, inputImg, upper_mask, lower_mask);

    // Create feature
    shared_ptr<Feature> feature = make_shared<Feature>(instance_info, 1);
    feature->addHistogram(*u_hist);
    feature->addHistogram(*l_hist);

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

    QElapsedTimer timer;
    timer.start();

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
                Skeleton::convertDepthCoordinatesToJoint(j+depthFrame.offset()[0], i+depthFrame.offset()[1],
                        depth[j], &point[0], &point[1]);

                /*m_device->convertDepthCoordinatesToJoint(j+depthFrame.offset()[0], i+depthFrame.offset()[1],
                        depth[j], &point[0], &point[1]);*/

                SkeletonJoint closerJoint = getCloserJoint(point, joints);
                mask[j] = closerJoint.getType()+1;
            }
        }
    }

    qDebug() << "Voronoi Cells" << timer.elapsed();

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
                Point3f point(0, 0, depth[j]);
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
    foreach (SkeletonJoint joint, joints) {
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
    float minDistance = 9999999999; //numeric_limits<float>::max();

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

void PersonReid::makeUpJoints(Skeleton& skeleton) const
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
