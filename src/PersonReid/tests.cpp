#include "tests.h"
#include "opencv_utils.h"
#include "types/ColorFrame.h"
#include "types/DepthFrame.h"
#include <QFile>
#include <QCryptographicHash>
#include "viewer/InstanceViewerWindow.h"
#include "dataset/DAI4REID_Parsed/DAI4REID_Parsed.h"
#include "PersonReid.h"
#include "ml/KMeans.h"


namespace dai {

// Test: save image as binary and read it
void Tests::test1()
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

void Tests::test2()
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

void Tests::test3()
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

void Tests::show_different_skel_resolutions()
{
    Dataset* dataset = new DAI4REID_Parsed;
    dataset->setPath("/files/DAI4REID_Parsed");

    const DatasetMetadata& metadata = dataset->getMetadata();
    shared_ptr<InstanceInfo> instance_info = metadata.instance(3, 1, 595, {});

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

    shared_ptr<Skeleton> skeleton[4];
    skeleton[0] = skeletonFrame->getSkeleton(skeletonFrame->getAllUsersId().at(0));

    for (int i=1; i<4; ++i) {
        skeleton[i] = make_shared<Skeleton>();
        *skeleton[i] = *skeleton[0];
    }

    // Improve skeleton
    PersonReid::makeUpJoints(*skeleton[1], true);   // R1
    PersonReid::makeUpOnlySomeJoints(*skeleton[2]); // R2
    PersonReid::makeUpJoints(*skeleton[3], false);  // R3

    // Get voronoi for each region and draw it in the color frame
    shared_ptr<MaskFrame> voronoi[4];
    shared_ptr<ColorFrame> color[4];
    cv::Mat color_mat[4];

    for (int i=0; i<4; ++i) {
        voronoi[i] = PersonReid::getVoronoiCells(*depthFrame, *maskFrame, *skeleton[i]);
        color[i] = make_shared<ColorFrame>(colorFrame->width(), colorFrame->height());
        PersonReid::colorImageWithVoronoid(*color[i], *voronoi[i]);
        color_mat[i] = cv::Mat(color[i]->height(), color[i]->width(), CV_8UC3,
                          (void*) color[i]->getDataPtr(), color[i]->getStride());

        cv::imshow("test" + QString::number(i).toStdString(), color_mat[i]);
        cv::imwrite("/home/jpadilla/sample-test-" + QString::number(i).toStdString() + ".png", color_mat[i]);
    }


    cv::waitKey(30000);


    // Close Instances
    instance->close();
}

// Approach 1: log color space (2 channels) without Histogram!!
// Paper: Color Invariants for Person Reidentification
void Tests::approach1(QHashDataFrames &frames)
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
void Tests::approach2(QHashDataFrames &frames)
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
void Tests::approach3(QHashDataFrames& frames)
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
        u_hist->print(n_often_colors);

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
void Tests::approach4(QHashDataFrames &frames)
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
void Tests::approach5(QHashDataFrames& frames)
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
void Tests::approach6(QHashDataFrames &frames)
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
