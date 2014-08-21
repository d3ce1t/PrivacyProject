#include "PersonReid.h"
#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include "dataset/CAVIAR4REID/CAVIAR4REID.h"
#include "viewer/InstanceViewerWindow.h"
#include "Config.h"
#include "opencv_utils.h"
#include "types/Histogram.h"
#include <QThread>

using namespace std;

namespace dai {


void PersonReid::execute()
{
    Dataset* dataset = new CAVIAR4REID;
    dataset->setPath("C:/datasets/CAVIAR4REID");
    const DatasetMetadata& metadata = dataset->getMetadata();

    InstanceViewerWindow viewer;
    viewer.show();

    QList<shared_ptr<Feature>> samples;
    QList<shared_ptr<Feature>> actor_samples;
    QList<shared_ptr<Feature>> centroids;
    const int num_actors = 50;

    //
    // Training
    //

    // For each actor 1-10, compute the feature that minimises the distance to each other sample of
    // the same actor.
    for (int i=1; i<=num_actors; ++i)
    {
        QList<shared_ptr<InstanceInfo>> instances = metadata.instances(DataFrame::Color,
                                                                       {i}, {1},
                                                                       QList<QList<QString>>());

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
            instance->readNextFrame();
            shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(instance->frames().at(0));

            // Feature Extraction
            shared_ptr<Feature> feature = featureExtraction(colorFrame, *instance_info);
            samples << feature;
            actor_samples << feature;

            // Output
            viewer.showFrame(colorFrame);
            //cv::imshow("Hist", distImg);

            // Process Events and sleep
            std::fflush(stdout);
            QCoreApplication::processEvents();
            //cv::waitKey(600);
            //QThread::msleep(1);
        }

        shared_ptr<Feature> selectedFeature = Feature::minFeature(actor_samples);
        centroids << selectedFeature;
        /*printf("Init. Centroid %i = actor %i %i\n", i, selectedFeature->label().getActor(),
                                                       selectedFeature->label().getSample());*/
    }

    // Learn a feature for each actor (the centroid of the cluster)
    auto kmeans = KMeans<Feature>::execute(samples, num_actors, centroids);
    //printClusters(kmeans->getClusters());

    QList<shared_ptr<Feature>> gallery;

    foreach (auto cluster, kmeans->getClusters()) {
        gallery << cluster.centroid;
    }

    // Validation
    QList<shared_ptr<InstanceInfo>> instances = metadata.instances(DataFrame::Color,
                                                                   {1,2,3,4,5,6,7,8,9,10},
                                                                   {1},
                                                                   DatasetMetadata::ANY_LABEL);
    int results[num_actors];
    memset(results, 0, sizeof(int) * num_actors);

    foreach (shared_ptr<InstanceInfo> instance_info, instances)
    {
        // Get Sample
        shared_ptr<StreamInstance> instance = dataset->getInstance(*instance_info, DataFrame::Color);
        instance->open();
        instance->readNextFrame();
        shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(instance->frames().at(0));

        // Feature Extraction
        shared_ptr<Feature> query = featureExtraction(colorFrame, *instance_info);

        // Compute distance to all samples in gallery
        QMap<float, int> query_results;

        foreach (shared_ptr<Feature> target, gallery) {
            float distance = query->distance(*target);
            query_results.insert(distance, target->label().getActor());
        }

        // Count results as a CMC
        auto it = query_results.constBegin();
        int pos = -1;
        int i = 0;

        // Find position
        while (it != query_results.constEnd() && pos == -1) {
            if (it.value() == query->label().getActor()) {
                pos = i;
            }
            ++it;
            ++i;
        }

        // Accumulate for ranks
        for (int i=pos; i<num_actors; ++i) {
            results[i]++;
        }

        /*i = 0;
        cout << "Results for actor " << instance_info->getActor() << " " << instance_info->getSample() << endl;
        for (auto it = query_results.constBegin(); it != query_results.constEnd(); ++it) {
            if (i == pos) cout << "*";
            cout << "dist " << it.key() << " actor " << it.value() << endl;
            ++i;
        }
        cout << "--------------------------" << endl;*/
    }

    // Show Results
    cout << "Rank" << "\t" << "Matching Rate" << endl;
    for (int i=0; i<num_actors; ++i) {
        cout << (i+1) << "\t" << results[i] << endl;
    }

    QCoreApplication::instance()->quit();
}

shared_ptr<Feature> PersonReid::featureExtraction(shared_ptr<ColorFrame> colorFrame, const InstanceInfo& instance_info) const
{
    cv::Mat inputImg(colorFrame->getHeight(), colorFrame->getWidth(),
                 CV_8UC3, (void*)colorFrame->getDataPtr(), colorFrame->getStride());

    cv::Mat upper_mask, lower_mask;
    dai::computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask);

    // Color Palette with 16-16-16 levels
    cv::Mat indexedImg = dai::convertRGB2Indexed161616(inputImg);

    // Compute the histogram for the upper (torso) and lower (leggs) parts of each frame in the buffer
    auto u_hist = Histogram1s::create(indexedImg, {0, 4095}, upper_mask);
    auto l_hist = Histogram1s::create(indexedImg, {0, 4095}, lower_mask);

    dai::colorImageWithMask(inputImg, inputImg, upper_mask, lower_mask);

    return make_shared<Feature>(*u_hist, *l_hist, instance_info);
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
