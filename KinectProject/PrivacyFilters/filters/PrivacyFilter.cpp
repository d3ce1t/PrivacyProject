#include "PrivacyFilter.h"
#include "types/MaskFrame.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "viewer/SilhouetteItem.h"
#include "viewer/SkeletonItem.h"
#include "types/MetadataFrame.h"
#include <QDebug>
#include <QThread>
#include <QElapsedTimer>
#include <iostream>
#include <cmath>
#include <QList>
#include <boost/random.hpp>
#include "Utils.h"
#include "ml/KMeans.h"
#include <cmath>
#include "opencv_utils.h"

namespace dai {

PrivacyFilter::PrivacyFilter()
    : m_glContext(nullptr)
    , m_gles(nullptr)
    , m_initialised(false)
    , m_fboDisplay(nullptr)
    , m_filter(FILTER_DISABLED)
    , m_file("data.csv")
    , m_out(&m_file)
{
    QSurfaceFormat format;
    format.setMajorVersion(2);
    format.setMinorVersion(0);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::SingleBuffer);

    m_surface.setFormat(format);
    m_surface.create();

    if (!m_surface.isValid()) {
        qDebug() << "The surface could not be created";
        throw 1;
    }

    m_scene = new Scene2DPainter;
    m_ogreScene = new OgreScene;

    // haarcascade_frontalface_default
    // haarcascade_frontalface_alt.xml
    /*if (!m_face_cascade.load("haarcascade_frontalface_alt.xml"))
        qDebug() << "Error loading cascades";*/
}

PrivacyFilter::~PrivacyFilter()
{
   stopListener();
   freeResources();
   if (m_file.isOpen())
       m_file.close();
   qDebug() << "PrivacyFilter::~PrivacyFilter";
}

void PrivacyFilter::initialise()
{
    m_glContext = new QOpenGLContext;
    m_glContext->setFormat(m_surface.format());

    if (!m_glContext->create()) {
        qDebug() << "Could not create the OpenGL context";
        throw 1;
    }

    m_glContext->makeCurrent(&m_surface);
    m_gles = m_glContext->functions();
    m_ogreScene->initialise();
    m_scene->setAvatarTexture(m_ogreScene->texture());

    // Create frame buffer Object
    QOpenGLFramebufferObjectFormat format;
    format.setInternalTextureFormat(GL_RGB);
    format.setTextureTarget(GL_TEXTURE_2D);
    format.setSamples(0);
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

    m_fboDisplay = new QOpenGLFramebufferObject(QSize(640, 480), format);

    if (!m_fboDisplay->isValid()) {
        qDebug() << "FBO Error";
        throw 2;
    }

    m_glContext->doneCurrent();
    m_initialised = true;
}

void PrivacyFilter::freeResources()
{
    if (m_glContext)
    {
        m_glContext->makeCurrent(&m_surface);

        if (m_ogreScene) {
            delete m_ogreScene;
            m_ogreScene = nullptr;
        }

        if (m_scene) {
            delete m_scene;
            m_scene = nullptr;
        }

        if (m_fboDisplay) {
            m_fboDisplay->release();
            delete m_fboDisplay;
            m_fboDisplay = nullptr;
        }

        m_glContext->doneCurrent();

        delete m_glContext;
        m_glContext = nullptr;
    }

    m_gles = nullptr;
    m_initialised = false;
}

// This method is called from a thread from the PlaybackControl
void PrivacyFilter::newFrames(const QHashDataFrames dataFrames)
{
    if (!m_initialised) {
        initialise();
    }

    // Copy frames (1 ms)
    m_frames.clear();

    foreach (DataFrame::FrameType key, dataFrames.keys()) {
        shared_ptr<DataFrame> frame = dataFrames.value(key);
        m_frames.insert(key, frame->clone());
    }

    // Check if the frames has been copied correctly
    if (!hasExpired()) {
        bool newFrames = generate();
        if (subscribersCount() == 0 || !newFrames) {
            qDebug() << "PrivacyFilter: No listeners or Nothing produced";
            stopListener();
        }
    }
    else {
        qDebug() << "Frame has expired!";
    }
}

void PrivacyFilter::afterStop()
{
    freeResources();
}

void PrivacyFilter::enableFilter(ColorFilter filterType)
{
    m_filter = filterType;
}

// Approach 1: log color space (2 channels) without Histogram!!
// Paper: Color Invariants for Person Reidentification
void PrivacyFilter::approach1()
{
    Q_ASSERT(m_frames.contains(DataFrame::Color) && m_frames.contains(DataFrame::Mask) &&
             m_frames.contains(DataFrame::Skeleton) && m_frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(m_frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(m_frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(m_frames.value(DataFrame::Metadata));

    // Work in the Bounding Box
    if (metadataFrame->boundingBoxes().isEmpty())
        return;

    BoundingBox bb = metadataFrame->boundingBoxes().first();
    shared_ptr<ColorFrame> subColorFrame = colorFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> subMaskFrame = maskFrame->subFrame(bb.getMin().val(1),bb.getMin().val(0),
                                                             bb.size().width(), bb.size().height());

    // Use cv::Mat for my color frame and mask frame
    cv::Mat inputImg(subColorFrame->getHeight(), subColorFrame->getWidth(),
                 CV_8UC3, (void*)subColorFrame->getDataPtr(), subColorFrame->getStride());

    cv::Mat mask(subMaskFrame->getHeight(), subMaskFrame->getWidth(),
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
void PrivacyFilter::approach2()
{
    Q_ASSERT(m_frames.contains(DataFrame::Color) && m_frames.contains(DataFrame::Mask) &&
             m_frames.contains(DataFrame::Skeleton) && m_frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(m_frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(m_frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(m_frames.value(DataFrame::Metadata));

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
        Mat inputImg(subColorFrame->getHeight(), subColorFrame->getWidth(),
                     CV_8UC3, (void*)subColorFrame->getDataPtr(), subColorFrame->getStride());

        Mat mask(subMaskFrame->getHeight(), subMaskFrame->getWidth(),
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
        createHistDistImage<uchar,2>({u_hist.get(), l_hist.get()},
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
void PrivacyFilter::approach3()
{
    Q_ASSERT(m_frames.contains(DataFrame::Color) && m_frames.contains(DataFrame::Mask) &&
             m_frames.contains(DataFrame::Skeleton) && m_frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(m_frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(m_frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(m_frames.value(DataFrame::Metadata));

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
        Mat inputImg(subColorFrame->getHeight(), subColorFrame->getWidth(),
                     CV_8UC3, (void*)subColorFrame->getDataPtr(), subColorFrame->getStride());

        Mat mask(subMaskFrame->getHeight(), subMaskFrame->getWidth(),
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
        createHistDistImage<uchar,2>({u_hist.get(), l_hist.get()},
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
void PrivacyFilter::approach4()
{
    Q_ASSERT(m_frames.contains(DataFrame::Color) && m_frames.contains(DataFrame::Mask) &&
             m_frames.contains(DataFrame::Skeleton) && m_frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(m_frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(m_frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(m_frames.value(DataFrame::Metadata));

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
        Mat inputImg(roiColorFrame->getHeight(), roiColorFrame->getWidth(),
                     CV_8UC3, (void*)roiColorFrame->getDataPtr(), roiColorFrame->getStride());

        Mat mask(roiMaskFrame->getHeight(), roiMaskFrame->getWidth(),
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
        createHistDistImage<uchar,3>({u_hist.get(), l_hist.get()},
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
void PrivacyFilter::approach5()
{
    Q_ASSERT(m_frames.contains(DataFrame::Color) && m_frames.contains(DataFrame::Mask) &&
             m_frames.contains(DataFrame::Skeleton) && m_frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(m_frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(m_frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(m_frames.value(DataFrame::Metadata));

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
        Mat inputImg(roiColorFrame->getHeight(), roiColorFrame->getWidth(),
                     CV_8UC3, (void*)roiColorFrame->getDataPtr(), roiColorFrame->getStride());

        Mat mask(roiMaskFrame->getHeight(), roiMaskFrame->getWidth(),
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
            createHistDistImage<uchar,3>({&u_hist_acc, &l_hist_acc},
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
void PrivacyFilter::approach6()
{
    Q_ASSERT(m_frames.contains(DataFrame::Color) && m_frames.contains(DataFrame::Mask) &&
             m_frames.contains(DataFrame::Skeleton) && m_frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(m_frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(m_frames.value(DataFrame::Mask));
    shared_ptr<MetadataFrame> metadataFrame = static_pointer_cast<MetadataFrame>(m_frames.value(DataFrame::Metadata));

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
        Mat inputImg(roiColorFrame->getHeight(), roiColorFrame->getWidth(),
                     CV_8UC3, (void*)roiColorFrame->getDataPtr(), roiColorFrame->getStride());

        Mat mask(roiMaskFrame->getHeight(), roiMaskFrame->getWidth(),
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
                    createHistDistImage<uchar,1>({hist.get()}, {Scalar(0, 0, 255)}, hist_img[i]);
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

template <typename T, int N>
bool PrivacyFilter::compare(const cv::Mat& inputImg, const QList<Point<T,N>>& point_list, const cv::Mat& mask)
{
    Q_ASSERT( (mask.rows == 0 && mask.cols == 0) || (mask.rows == inputImg.rows && mask.cols == inputImg.cols) );

    auto it = point_list.constBegin();
    bool useMask = mask.rows > 0 && mask.cols > 0;
    bool equal = true;

    if (useMask) {
        if ( count_pixels_nz<uchar>(mask) != point_list.size())
            return false;
    } else {
        if (inputImg.rows * inputImg.cols != point_list.size())
            return false;
    }

    for (int i=0; i<inputImg.rows; ++i)
    {
        const cv::Vec<T,N>* pixel = inputImg.ptr<cv::Vec<T,N>>(i);
        const uchar* pMask = useMask ? mask.ptr<uchar>(i) : nullptr;

        for (int j=0; j<inputImg.cols; ++j)
        {
            if (useMask && pMask[j] <= 0)
                continue;

            const Point<T,N>& point = *it;
            int k = 0;

            while (k < N && equal)
            {
                equal = point[k] == pixel[j][k];
                ++k;
            }

            ++it;
            if (!equal) break;
        }

        if (!equal) break;
    }

    return equal;
}

template <class T, int N>
void PrivacyFilter::printHistogram(const Histogram<T, N> &hist, int n_elems) const
{
    // Show info of the histogram
    int i=0;
    foreach (auto item, hist.higherFreqBins(n_elems)) {
        qDebug() << i++ << "(" << item->point.toString() << ")" << item->value << item->dist;
    }
}

void PrivacyFilter::denoiseImage(cv::Mat input_img, cv::Mat output_img) const
{
    // without filter: u.min 7 u.max 4466 u.nz 505 l.min 0 l.max 1473 l.nz 331
    // filter:         u.min 7 u.max 3976 u.nz 577 l.min 0 l.max 1490 l.nz 323
    medianBlur(input_img, output_img, 7);
    //GaussianBlur(input_img, output_img, cv::Size(7, 7), 0, 0);
    //GaussianBlur(output_img, output_img, cv::Size(7, 7), 0, 0);
    //bilateralFilter(image, denoiseImage, 9, 18, 4.5f);
}

void PrivacyFilter::discretiseRGBImage(cv::Mat input_img, cv::Mat output_img) const
{
    using namespace cv;

    const int module = 5;

    for (int i=0; i<input_img.rows; ++i)
    {
        Vec3b* in_pixel = input_img.ptr<Vec3b>(i);
        Vec3b* out_pixel = output_img.ptr<Vec3b>(i);

        for (int j=0; j<input_img.cols; ++j)
        {
            int rest_r = in_pixel[j][0] % module;
            int rest_g = in_pixel[j][1] % module;
            int rest_b = in_pixel[j][2] % module;

            if (rest_r > 0) {
                out_pixel[j][0] = in_pixel[j][0] + (module - rest_r);
            } else {
                out_pixel[j][0] = in_pixel[j][0];
            }

            if (rest_g > 0) {
                out_pixel[j][1] = in_pixel[j][1] + (module - rest_g);
            } else {
                out_pixel[j][1] = in_pixel[j][1];
            }

            if (rest_b > 0) {
                out_pixel[j][2] = in_pixel[j][2] + (module - rest_b);
            } else {
                out_pixel[j][2] = in_pixel[j][2];
            }
        }
    }
}

QHashDataFrames PrivacyFilter::produceFrames()
{
    Q_ASSERT(m_frames.contains(DataFrame::Color) && m_frames.contains(DataFrame::Mask) &&
             m_frames.contains(DataFrame::Skeleton) && m_frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(m_frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(m_frames.value(DataFrame::Mask));

    //approach1();
    //approach2();
    //approach3();
    //approach4();
    //approach5();
    approach6();

    // Dilate mask to create a wide border (value = 255)
    /*shared_ptr<MaskFrame> inputMask = static_pointer_cast<MaskFrame>(m_frames.value(DataFrame::Mask));
    shared_ptr<MaskFrame> outputMask = static_pointer_cast<MaskFrame>(inputMask->clone());
    dilateUserMask(const_cast<uint8_t*>(outputMask->getDataPtr()));

    for (int i=0; i<inputMask->getHeight(); ++i)
    {
        for (int j=0; j<inputMask->getWidth(); ++j)
        {
            uint8_t inputValue = inputMask->getItem(i,j);
            uint8_t outputValue = outputMask->getItem(i,j);

            if (inputValue == 0 && outputValue > 0) {
                inputMask->setItem(i, j, uint8_t(255));
            }
        }
    }*/

    //
    // Prepare Scene
    //
    shared_ptr<SilhouetteItem> silhouetteItem = static_pointer_cast<SilhouetteItem>(m_scene->getFirstItem(ITEM_SILHOUETTE));
    shared_ptr<SkeletonItem> skeletonItem = static_pointer_cast<SkeletonItem>(m_scene->getFirstItem(ITEM_SKELETON));

    // Clear items of the scene
    m_scene->clearItems();

    // Background of Scene
    m_scene->setBackground(m_frames.value(DataFrame::Color));
    m_scene->setMask(maskFrame);

    // Add silhuette item to the scene
    if (!silhouetteItem)
        silhouetteItem.reset(new SilhouetteItem);

    silhouetteItem->setUser(maskFrame);
    m_scene->addItem(silhouetteItem);

    // Add skeleton item to the scene
    if (!skeletonItem)
        skeletonItem.reset(new SkeletonItem);

    skeletonItem->setSkeleton( static_pointer_cast<SkeletonFrame>(m_frames.value(DataFrame::Skeleton)) );
    m_scene->addItem(skeletonItem);

    // Enable Filter
    m_scene->enableFilter(m_filter);
    if (m_filter == FILTER_3DMODEL)
        m_ogreScene->enableFilter(true);
    else
        m_ogreScene->enableFilter(false);

    // Prepare Data of the OgreScene
    m_ogreScene->prepareData(m_frames);
    m_scene->markAsDirty();

    //
    // Render scene offscreen
    //

    // Render Avatar
    m_ogreScene->render();

    // Render and compose rest of the scene
    m_glContext->makeCurrent(&m_surface);
    m_scene->setSize(m_fboDisplay->width(), m_fboDisplay->height());
    m_scene->renderScene(m_fboDisplay);

    m_fboDisplay->bind();

    // Copy data back to ColorFrame
    m_gles->glReadPixels(0,0, m_fboDisplay->width(), m_fboDisplay->height(), GL_RGB, GL_UNSIGNED_BYTE,
                         (GLvoid*) colorFrame->getDataPtr());

    m_fboDisplay->release();
    m_glContext->doneCurrent();

    return m_frames;
}

cv::Mat PrivacyFilter::createMask(cv::Mat input_img, int min_value, int* nonzero_counter, bool filter) const
{
    using namespace cv;

    Mat output_mask = Mat::zeros(input_img.rows, input_img.cols, CV_8UC1);
    int counter = 0;

    for (int i=0; i<input_img.rows; ++i)
    {
        float* pixel = input_img.ptr<float>(i);

        for (int j=0; j<input_img.cols; ++j)
        {
            if (pixel[j] > min_value) {
                output_mask.at<uchar>(i, j) = 1;
                counter++;
            } else if (filter){
                pixel[j] = 0;
            }
        }
    }

    if (nonzero_counter)
        *nonzero_counter = counter;

    return output_mask;
}

cv::Mat PrivacyFilter::convertRGB2Log2DAsMat(const cv::Mat &inputImg)
{
    cv::Mat outputImg = cv::Mat::zeros(inputImg.rows, inputImg.cols, CV_32FC2);

    for (int i=0; i<inputImg.rows; ++i)
    {
        const cv::Vec3b* inPixel = inputImg.ptr<cv::Vec3b>(i);
        cv::Vec2f* outPixel = outputImg.ptr<cv::Vec2f>(i);

        for (int j=0; j<inputImg.cols; ++j) {
            // Approach: Consider RGB colors from 1 to 256
            // min color: log(1/256) ~ -5.6
            // max color: log(256/1) ~ 5.6
            outPixel[j][0] = std::log( float(inPixel[j][0]+1) / float(inPixel[j][1]+1) ); // log ( R/G )
            outPixel[j][1] = std::log( float(inPixel[j][2]+1) / float(inPixel[j][1]+1) ); // log ( B/G )
        }
    }

    return outputImg;
}

QList<Point2f> PrivacyFilter::convertRGB2Log2DAsList(const QList<Point3b>& list)
{
    QList<Point2f> result;

    for (auto it = list.constBegin(); it != list.constEnd(); ++it)
    {
        const Point3b& point = *it;
        float log_rg = std::log10( float( point[0]+1 ) / float( point[1]+1 ) ); // log ( R/G )
        float log_bg = std::log10( float( point[2]+1 ) / float( point[1]+1 ) ); // log ( B/G )
        result.append( Point2f(log_rg, log_bg) );
    }

    return result;
}

template <class T>
int PrivacyFilter::count_pixels_nz(const cv::Mat& inputImg)
{
    Q_ASSERT(inputImg.channels() == 1);

    int counter = 0;

    for (int i=0; i<inputImg.rows; ++i) {
        const T* pixel = inputImg.ptr<T>(i);
        for (int j=0; j<inputImg.cols; ++j) {
            if (pixel[j] > 0)
                counter++;
        }
    }

    return counter;
}

template <class T, int N>
cv::Mat PrivacyFilter::randomSamplingAsMat(const cv::Mat &inputImg, int n, const cv::Mat &mask)
{
    Q_ASSERT( (mask.rows == 0 && mask.cols == 0) || (mask.rows == inputImg.rows && mask.cols == inputImg.cols) );

    using namespace cv;

    Mat sampledImage = Mat::zeros(inputImg.rows, inputImg.cols, inputImg.type());
    QSet<int> used_samples;
    int k = inputImg.rows * inputImg.cols - 1;
    bool useMask = mask.rows > 0 && mask.cols > 0;
    int n_mask_pixels = 0;

    boost::mt19937 generator;
    generator.seed(time(0));
    boost::uniform_int<> uniform_dist(0, k);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<>> uniform_rnd(generator, uniform_dist);

    if (useMask) {
        // There are no guarantee that n pixels will sample
        n = dai::min<int>(n, count_pixels_nz<uchar>(mask));
    }

    n = dai::min<int>(n, k);

    int i = 0;
    int attempts = 0;

    while (i < n)
    {
        int z = uniform_rnd();

        if (!used_samples.contains(z)) {

            int row = z / inputImg.cols;
            int col = z % inputImg.cols;

            if (useMask && mask.at<uchar>(row,col) <= 0) {
                attempts++;
                continue;
            }

            sampledImage.at<Vec<T,N>>(row,col) = inputImg.at<Vec<T,N>>(row,col);
            used_samples << z;
            i++;
            attempts = 0;
        }
        else {
            attempts++;
        }

        if (attempts > 0 && attempts % 1000 == 0) {
            qDebug() << "Privacy Filter Stalled" << "attempts" << attempts << "i" << i << "n" << n << "k" << k << "mask pixels" << n_mask_pixels;
        }
    }

    return sampledImage;
}

template <class T, int N>
QList<Point<T,N>> PrivacyFilter::randomSampling(const cv::Mat &inputImg, int n, const cv::Mat &mask)
{
    Q_ASSERT( (mask.rows == 0 && mask.cols == 0) || (mask.rows == inputImg.rows && mask.cols == inputImg.cols) );

    QList<Point<T,N> > result;
    bool useMask = mask.rows > 0 && mask.cols > 0;
    int n_mask_pixels = 0;
    QSet<int> used_samples;
    int k = inputImg.rows * inputImg.cols - 1;

    boost::mt19937 generator;
    generator.seed(time(0));
    boost::uniform_int<> uniform_dist(0, k);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<>> uniform_rnd(generator, uniform_dist);

    if (useMask) {
        // There are no guarantee that n pixels will sample
        n = dai::min<int>(n, count_pixels_nz<uchar>(mask));
    }

    n = dai::min<int>(n, k);

    int i = 0;
    int attempts = 0;

    while (i < n)
    {
        int z = uniform_rnd();

        if (!used_samples.contains(z)) {

            int row = z / inputImg.cols;
            int col = z % inputImg.cols;

            if (useMask && mask.at<uchar>(row,col) <= 0) {
                attempts++;
                continue;
            }

            Point<T,N> point;
            const cv::Vec<T,N>& pixel = inputImg.at<cv::Vec<T,N>>(row,col);
            for (int j=0; j<N; ++j) {
                point[j] = pixel[j];
            }

            result.append(point);
            used_samples << z;
            i++;
            attempts = 0;
        }
        else {
            attempts++;
        }

        if (attempts > 0 && attempts % 1000 == 0) {
            qDebug() << "Privacy Filter Stalled" << "attempts" << attempts << "i" << i << "n" << n << "k" << k << "mask pixels" << n_mask_pixels;
        }
    }

    return result;
}

template <class T, int N>
cv::Mat PrivacyFilter::samplingAsMat(const cv::Mat &inputImg, const cv::Mat &mask)
{
    Q_ASSERT( (mask.rows == 0 && mask.cols == 0) || (mask.rows == inputImg.rows && mask.cols == inputImg.cols) );

    using namespace cv;

    Mat sampledImage = Mat::zeros(inputImg.rows, inputImg.cols, inputImg.type());
    bool useMask = mask.rows > 0 && mask.cols > 0;

    for (int i=0; i<inputImg.rows; ++i)
    {
        const Vec<T,N>* pixel = inputImg.ptr<Vec<T,N>>(i);
        Vec<T,N>* outPixel = sampledImage.ptr<Vec<T,N>>(i);
        const uchar* pMask = useMask ? mask.ptr<uchar>(i) : nullptr;

        for (int j=0; j<inputImg.cols; ++j)
        {
            if (useMask && pMask[j] <= 0)
                continue;

            outPixel[j] = pixel[j];
        }
    }

    return sampledImage;
}

template <class T, int N>
QList<Point<T,N>> PrivacyFilter::samplingAsList(const cv::Mat& inputImg, const cv::Mat& mask)
{
    Q_ASSERT( (mask.rows == 0 && mask.cols == 0) || (mask.rows == inputImg.rows && mask.cols == inputImg.cols) );

    QList<Point<T,N> > result;
    bool useMask = mask.rows > 0 && mask.cols > 0;

    for (int i=0; i<inputImg.rows; ++i)
    {
        const cv::Vec<T,N>* pixel = inputImg.ptr<cv::Vec<T,N>>(i);
        const uchar* pMask = useMask ? mask.ptr<uchar>(i) : nullptr;

        for (int j=0; j<inputImg.cols; ++j)
        {
            if (useMask && pMask[j] <= 0)
                continue;

            Point<T,N> point;
            for (int k=0; k<N; ++k)
                point[k] = pixel[j][k];
            result.append(point);
        }
    }

    return result;
}

template <class T>
void PrivacyFilter::create2DColorPalette(const QList<const HistBin3D<T>*>& upper_hist, const QList<const HistBin3D<T>*>& lower_hist, cv::Mat& output_img) const
{
    using namespace cv;

    int width = 400;
    int height = 400;
    const int num_items = dai::max<int>(upper_hist.size(), lower_hist.size());

    int size = cvCeil(std::sqrt(num_items * 2));
    int size_offset = size % 2;

    const int num_rows = size + size_offset;
    const int num_cols = num_rows;
    const int total_cells = num_rows * num_cols;
    float color_height = float(height) / num_rows;
    float color_width = float(width) / num_cols;
    int row = 0, col = 0;

    output_img = Mat::zeros(height, width, CV_8UC3);

    for (int i=0; i<400; ++i)
    {
        Vec3b* pixel = output_img.ptr<Vec3b>(i);
        row = i / color_height;

        for (int j=0; j<400; ++j)
        {
            col = j / color_width;

            if (row < num_rows / 2) {
                int offset = row * num_rows + col;
                if (offset < upper_hist.size()) {
                    const HistBin3D<T>* item = upper_hist.at(offset);
                    pixel[j][0] = item->point[0]; // Y Blue
                    pixel[j][1] = item->point[1]; // u Green
                    pixel[j][2] = item->point[2]; // v Red
                }
            } else {
                int offset = (row * num_rows + col) - (total_cells / 2);
                if (offset < lower_hist.size()) {
                    const HistBin3D<T>* item = lower_hist.at(offset);
                    pixel[j][0] = item->point[0]; // Y Blue
                    pixel[j][1] = item->point[1]; // u Green
                    pixel[j][2] = item->point[2]; // v Red
                }
            }
        }
    }
}

template <class T>
void PrivacyFilter::createHistImage(const QList<const HistBin3D<T>*>& hist_items, cv::Mat& output_img) const
{
    using namespace cv;

    int width = 400;
    int height = 400;
    const int num_items = hist_items.size();
    int size = cvCeil(std::sqrt(num_items));
    int size_offset = size % 2;

    const int num_rows = size + size_offset;
    const int num_cols = num_rows;
    const int total_cells = num_rows * num_cols;
    float color_height = float(height) / num_rows;
    float color_width = float(width) / num_cols;
    int row = 0, col = 0;

    output_img = Mat::zeros(height, width, CV_8UC3);

    for (int i=0; i<400; ++i)
    {
        Vec3b* pixel = output_img.ptr<Vec3b>(i);
        row = i / color_height;

        for (int j=0; j<400; ++j)
        {
            col = j / color_width;
            int offset = row * num_rows + col;

            if (offset < num_items) {
                const HistBin3D<T>* item = hist_items.at(offset);
                pixel[j][0] = item->point[0]; // Y Blue
                pixel[j][1] = item->point[1]; // u Green
                pixel[j][2] = item->point[2]; // v Red
            }
        }
    }
}

template <class T>
void PrivacyFilter::create2DCoordImage(const QList<Histogram2D<T>*>& hist_list, int n_items, const QList<cv::Vec3b> &color_list, cv::Mat& output_img,
                                       float input_range[]) const
{
    Q_ASSERT(hist_list.size() == color_list.size());

    using namespace cv;

    const int width = 400;
    const int height = 400;

    output_img = Mat(height, width, CV_8UC3, Scalar(0,0,0));

    int color_idx = 0;

    for (auto it = hist_list.constBegin(); it != hist_list.constEnd(); ++it)
    {
        Vec3b color = color_list.at(color_idx);
        auto items = (*it)->higherFreqBins(n_items);

        foreach (auto item, items)
        {
            float coord_y = dai::normalise<T>(item->point[0], input_range[0], input_range[1], 0, height);
            float coord_x = dai::normalise<T>(item->point[1], input_range[0], input_range[1], 0, width);
            output_img.at<Vec3b>(coord_y, coord_x) = color;
        }

        ++color_idx;
    }
}

void PrivacyFilter::create2DCoordImage(const QList<QList<Point2f>*> &input_list, const QList<cv::Vec3b>& color_list,
                                       cv::Mat& output_img, float input_range[]) const
{
    Q_ASSERT(input_list.size() == color_list.size());

    using namespace cv;

    const int width = 500;
    const int height = 500;

    output_img = Mat(height, width, CV_8UC3, Scalar(0,0,0));

    int color_idx = 0;

    for (auto it = input_list.constBegin(); it != input_list.constEnd(); ++it)
    {
        Vec3b color = color_list.at(color_idx);
        QList<Point2f>* items = *it;

        foreach (auto point, *items)
        {
            float coord_x = dai::normalise<float>(point[0], input_range[0], input_range[1], 0, width);
            float coord_y = dai::normalise<float>(point[1], input_range[0], input_range[1], 0, height);
            output_img.at<Vec3b>(coord_y, coord_x) = color;
        }

        ++color_idx;
    }
}

template <class T>
void PrivacyFilter::create2DCoordImage(cv::Mat input_img, cv::Mat& output_img, int size[], float input_range[], bool init_output, cv::Vec3b color) const
{
    Q_ASSERT(input_img.channels() == 2);
    Q_ASSERT( init_output == true || (output_img.rows == size[0] && output_img.cols == size[1]) );

    using namespace cv;

    if (init_output)
        output_img = Mat::zeros(size[0], size[1], CV_8UC3);

    for (int i=0; i<input_img.rows; ++i)
    {
        T* pixel = input_img.ptr<T>(i);

        for (int j=0; j<input_img.cols; ++j)
        {
            float coord_y = dai::normalise<float>(pixel[j][0], input_range[0], input_range[1], 0, size[0]);
            float coord_x = dai::normalise<float>(pixel[j][1], input_range[0], input_range[1], 0, size[1]);
            output_img.at<Vec3b>(coord_y, coord_x) = color;
        }
    }
}

void PrivacyFilter::dilateUserMask(uint8_t *labels)
{
    int dilationSize = 15;
    cv::Mat newImag(480, 640, cv::DataType<uint8_t>::type, labels);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS,
                                               cv::Size(2*dilationSize + 1, 2*dilationSize+1),
                                               cv::Point( dilationSize, dilationSize ) );
    cv::dilate(newImag, newImag, kernel);
}

std::vector<cv::Rect> PrivacyFilter::faceDetection(cv::Mat frameGray, bool equalised)
{
    using namespace cv;
    std::vector<Rect> faces;

    if (!equalised) {
        equalizeHist( frameGray, frameGray );
    }

    // Detect faces
    //m_face_cascade.detectMultiScale( frameGray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30) );

    return faces;
}

cv::Mat PrivacyFilter::calcHistogram(shared_ptr<ColorFrame> colorFrame, shared_ptr<MaskFrame> maskFrame = nullptr)
{
    using namespace cv;

    // Use my data with OpenCV Mat
    Mat inputImg = Mat(colorFrame->getHeight(), colorFrame->getWidth(),
                    CV_8UC3, (void*)colorFrame->getDataPtr(), colorFrame->getStride());
    Mat mask;

    // Set Mask if it exists
    if (maskFrame) {
        mask = Mat(maskFrame->getHeight(), maskFrame->getWidth(), CV_8UC1, (void*)maskFrame->getDataPtr(), maskFrame->getStride());
    }

    // Convert from RGB to CIE L*a*b*
    // L = [0, 100] -> [0,255]
    // a = [-127, 127] -> [0, 255]
    // b = [-127, 127] -> [0, 255]
    Mat image;
    cvtColor(inputImg, image, CV_RGB2Lab);

    // Image denoising
    Mat denoiseImage;
    GaussianBlur(image, denoiseImage, cv::Size(7, 7), 0, 0);
    //bilateralFilter(image, denoiseImage, 9, 18, 4.5f);

    // Split image in L*, a* and b* channels
    vector<Mat> lab_planes;
    split(denoiseImage, lab_planes);

    // Create interleave image
    Mat intImage = interleaveMatChannels<Vec3b>(denoiseImage, mask);
    normalize(intImage, intImage, 0, 65536, NORM_MINMAX, -1, Mat());
    intImage.convertTo(intImage, CV_16UC1);

    // Establish the number of bins
    int histSize = 256;

    // Set the ranges ( for L,a,b) )
    float range[] = {0, 256};
    float myRange[] = {0, 65536};
    const float* histRange = { range };
    const float* myHistRanges = { myRange };
    bool uniform = true;
    bool accumulate = false;

    // Compute the histograms:
    Mat l_hist, a_hist, b_hist, int_hist;
    calcHist( &lab_planes[0], 1, 0, mask, l_hist, 1, &histSize, &histRange, uniform, accumulate );
    calcHist( &lab_planes[1], 1, 0, mask, a_hist, 1, &histSize, &histRange, uniform, accumulate );
    calcHist( &lab_planes[2], 1, 0, mask, b_hist, 1, &histSize, &histRange, uniform, accumulate );
    calcHist( &intImage, 1, 0, mask, int_hist, 1, &histSize, &myHistRanges, uniform, accumulate);

    // Compute histogram for each row
    int numBins = 1;
    int numLines = cvCeil(denoiseImage.rows / numBins);
    if (denoiseImage.rows % numBins > 0) numBins++;
    Mat h_hist = Mat::zeros(numLines, 256, CV_16UC1);

    for (int i=0; i<denoiseImage.rows; ++i) {
        Vec3b* pixel = denoiseImage.ptr<Vec3b>(i);
        uchar* maskPixel = mask.ptr<uchar>(i);
        int lineIndex = i / numBins;
        for (int j=0; j<image.cols; ++j) {
            if (maskPixel[j] > 0) {
                h_hist.at<ushort>(lineIndex, pixel[j][0]) = h_hist.at<ushort>(lineIndex, pixel[j][0]) + 1;
            }
        }
    }

    /*double minValue, maxValue;
    minMaxLoc(iwImage, &minValue, &maxValue);
    qDebug() << "min" << minValue << "max" << maxValue;*/

    // Draw the histograms for L, a and b
    int hist_w = 512; int hist_h = 400;
    int bin_w = cvRound( (double) hist_w/histSize );

    Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

    // Normalize the result to [ 0, histImage.rows ]
    normalize(l_hist, l_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
    normalize(a_hist, a_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
    normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
    normalize(int_hist, int_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
    normalize(h_hist, h_hist, 30000, 65535, NORM_MINMAX, -1, Mat());

    // Draw for each channel
    for( int i=1; i < histSize; i++ )
    {
        // This is OpenCV blue channel (Red in real because of BGR)
        line(histImage,
             cv::Point( bin_w*(i-1), hist_h - cvRound(l_hist.at<float>(i-1)) ) ,
             cv::Point( bin_w*(i), hist_h - cvRound(l_hist.at<float>(i)) ),
             Scalar( 0, 0, 255), 1, 8, 0  );
        line(histImage,
             cv::Point( bin_w*(i-1), hist_h - cvRound(a_hist.at<float>(i-1)) ) ,
             cv::Point( bin_w*(i), hist_h - cvRound(a_hist.at<float>(i)) ),
             Scalar( 0, 255, 0), 1, 8, 0  );
        line(histImage,
             cv::Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
             cv::Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
             Scalar( 255, 0, 0), 1, 8, 0  );
        line(histImage,
             cv::Point( bin_w*(i-1), hist_h - cvRound(int_hist.at<float>(i-1)) ) ,
             cv::Point( bin_w*(i), hist_h - cvRound(int_hist.at<float>(i)) ),
             Scalar( 0, 255, 255), 1, 8, 0  );
    }

    // Display
    namedWindow("calcHist", CV_WINDOW_AUTOSIZE);
    imshow("calcHist", histImage);
    imshow("L-image", lab_planes[0]);
    imshow("a-image", lab_planes[1]);
    imshow("b-image", lab_planes[2]);
    imshow("Int-Image", intImage);
    imshow("Denoise-Image", denoiseImage);
    cv::waitKey(1);

    return Mat();
}

template <class T>
cv::Mat PrivacyFilter::interleaveMatChannels(cv::Mat inputMat, cv::Mat mask, int type)
{
    Q_ASSERT(inputMat.channels() == 2 || inputMat.channels() == 3);
    Q_ASSERT( (mask.rows == 0 && mask.cols == 0) || (mask.rows == inputMat.rows && mask.cols == inputMat.cols) );

    using namespace cv;

    bool useMask = mask.rows > 0 && mask.cols > 0;
    Mat intImage(inputMat.rows, inputMat.cols, type);
    int nChannels = inputMat.channels();

    for (int i=0; i<inputMat.rows; ++i)
    {
        T* inPixel = inputMat.ptr<T>(i);
        uchar* maskPixel = mask.ptr<uchar>(i);
        uint32_t* outPixel = intImage.ptr<uint32_t>(i);

        for (int j=0; j<inputMat.cols; ++j)
        {
            if (useMask && maskPixel[j] <= 0) {
                outPixel[j] = 0;
                continue;
            }

            T pixel_tmp = inPixel[j];
            uint32_t pixel24b = 0;

            for (int k=0; k<24; k+=3) {
                pixel24b |= (pixel_tmp[0] & 0x01) << k;
                pixel_tmp[0] >>= 1;

                pixel24b |= (pixel_tmp[1] & 0x01) << (k+1);
                pixel_tmp[1] >>= 1;

                if (nChannels == 3) {
                    pixel24b |= (pixel_tmp[2] & 0x01) << (k+2);
                    pixel_tmp[2] >>= 1;
                }
            }

            outPixel[j] = pixel24b;
        }
    }

    return intImage;
}

// Count number of pixels of the silhouette / number of pixels of the bounding box
double PrivacyFilter::computeOccupancy(shared_ptr<MaskFrame> mask, int* outNumPixels)
{
    double occupancy;
    int n_pixels = 0;

    for (int i=0; i<mask->getHeight(); ++i) {
        const uint8_t* pixelMask =  mask->getRowPtr(i);
        for (int j=0; j<mask->getWidth(); ++j) {
            if (pixelMask[j] > 0 && pixelMask[j] < 255) {
                n_pixels++;
            }
        }
    }

    occupancy = (float) n_pixels / (float) (mask->getHeight() * mask->getWidth());

    if (outNumPixels) {
        *outNumPixels = n_pixels;
    }

    return occupancy;
}

cv::Mat PrivacyFilter::computeIntegralImage(cv::Mat image)
{
    using namespace cv;
    Mat lookup = Mat::zeros(image.rows, image.cols, CV_32SC1);

    // Compute first row
    Vec3b* pixel = image.ptr<Vec3b>(0);
    uint32_t* value = lookup.ptr<uint32_t>(0);
    value[0] = pixel[0][0];

    for (int i=1; i<image.cols; ++i) {
        value[i] = pixel[i][0] + value[i-1];
    }

    // Compute first column
    for (int i=1; i<image.rows; ++i) {
        lookup.at<uint32_t>(i, 0) = image.at<Vec3b>(i, 0)[0] + lookup.at<uint32_t>(i-1, 0);
    }

    uint32_t* prevMatrixRow = value;

    // Compute Matrix
    for (int i=1; i<image.rows; ++i)
    {
        Vec3b* pPixelRow = image.ptr<Vec3b>(i);
        uint32_t* pMatrixRow = lookup.ptr<uint32_t>(i);

        for (int j=1; j<image.cols; ++j) {
            pMatrixRow[j] = pPixelRow[j][0] + pMatrixRow[j-1] + prevMatrixRow[j] - prevMatrixRow[j-1];
        }

        prevMatrixRow = pMatrixRow;
    }

    return lookup;
}

} // End Namespace
