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
#include <boost/random.hpp>
#include "Utils.h"

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

// Approach 1: log color space
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

    // Start OpenCV code
    {using namespace cv;

        // Use cv::Mat for my color frame and mask frame
        Mat inputImg(subColorFrame->getHeight(), subColorFrame->getWidth(),
                     CV_8UC3, (void*)subColorFrame->getDataPtr(), subColorFrame->getStep());

        Mat mask(subMaskFrame->getHeight(), subMaskFrame->getWidth(),
                 CV_8UC1, (void*)subMaskFrame->getDataPtr(), subMaskFrame->getStep());

        // Copy: because I'm going to modify inputImg to show some things on screen
        Mat copyImg = inputImg.clone();

        // Compute Upper and Lower Masks
        Mat upper_mask, lower_mask;
        computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask, mask); // This ones modifies inputImg

        // Sample Mask
        Mat upper_sampled_mask = randomSampling<uchar>(upper_mask, 200, upper_mask);
        Mat lower_sampled_mask = randomSampling<uchar>(lower_mask, 200, lower_mask);

        // Denoise Image
        denoiseImage(copyImg, copyImg);

        // Convert from RGB color space to 2D log color space
        Mat imgLog = convertRGB2Log2D(copyImg);

        // Compute the histogram for the upper (torso) and lower (leggs) parts
        auto u_hist = Histogram2D<float>::create(imgLog, upper_sampled_mask);
        auto l_hist = Histogram2D<float>::create(imgLog, lower_sampled_mask);

        // Show histogram info
        const auto u_max_item = u_hist->maxFreqItem();
        const auto l_max_item = l_hist->maxFreqItem();
        qDebug() << "u.max" << "(" << u_max_item.point[DIM_X] << u_max_item.point[DIM_Y] << ")" << u_max_item.value;
        qDebug() << "l.max" << "(" << l_max_item.point[DIM_X] << l_max_item.point[DIM_Y] << ")" << l_max_item.value;
        //qDebug() << "avg" << u_hist->avgFreq() << "n" << u_hist->numItems();

        // Show it on an image
        int img_size[] = {400, 400};
        float log_range[] = {-5.6f, 5.6f};
        //const float* hist_ranges[] = { log_range, log_range };
        Mat hist2DImg;
        create2DCoordImage(*u_hist, hist2DImg, img_size, log_range, true, Vec3b(255, 0, 0)); // Blue
        create2DCoordImage(*l_hist, hist2DImg, img_size, log_range, false, Vec3b(0, 0, 255)); // Red

        // Histogram of the histogram (Upper part & Lower part)
/*        int histSize = dai::max<float>(u_max, l_max);
        float histD_ranges[2]; histD_ranges[0] = dai::min(u_min, l_min);  histD_ranges[1] = dai::max<float>(u_max, l_max);
        const float* histD_range = {histD_ranges};
        Mat u_histD, l_histD;

        if (u_nonzero_counter)
            calcHist( &u_hist, 1, channels, u_hist_mask, u_histD, 1, &histSize, &histD_range, true, false);

        if (l_nonzero_counter)
            calcHist( &l_hist, 1, channels, l_hist_mask, l_histD, 1, &histSize, &histD_range, true, false);

        // Show it
        int bin_w = cvRound( (double) img_size[1]/histSize );
        Mat histImage( img_size[0], img_size[1], CV_8UC3, Scalar( 0,0,0) );

        // Normalize the result to [ 0, histImage.rows ]
        if (u_nonzero_counter)
            normalize(u_histD, u_histD, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

        if (l_nonzero_counter)
            normalize(l_histD, l_histD, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

        // Draw for each channel
        for( int i=1; i < histSize; i++)
        {
            // This is OpenCV blue channel (Red in real because of BGR)
            if (u_nonzero_counter)
                line(histImage,
                     Point( bin_w*(i-1), img_size[0] - cvRound(u_histD.at<float>(i-1)) ) ,
                        Point( bin_w*(i), img_size[0] - cvRound(u_histD.at<float>(i)) ),
                        Scalar( 0, 0, 255), 1, 8, 0  );

            if (l_nonzero_counter)
                line(histImage,
                     Point( bin_w*(i-1), img_size[0] - cvRound(l_histD.at<float>(i-1)) ) ,
                        Point( bin_w*(i), img_size[0] - cvRound(l_histD.at<float>(i)) ),
                        Scalar( 255, 0, 0), 1, 8, 0  );
        }
*/
        /*Mat u_hist_norm;
        divide(u_hist, u_max, u_hist_norm);

        Mat l_hist_norm;
        divide(l_hist, l_max, l_hist_norm);

        Mat diff_hist;
        absdiff(u_hist, l_hist, diff_hist);*/

        //normalize(u_hist, u_hist, 0,1, NORM_MINMAX, -1, Mat() );
        //normalize(l_hist, l_hist, 0,1, NORM_MINMAX, -1, Mat() );

        // Show
        imshow("hist2DImg", hist2DImg);
        waitKey(1);

    } // End OpenCV code

    return;
}

// Approach 2: CIElab
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
                     CV_8UC3, (void*)subColorFrame->getDataPtr(), subColorFrame->getStep());

        Mat mask(subMaskFrame->getHeight(), subMaskFrame->getWidth(),
                 CV_8UC1, (void*)subMaskFrame->getDataPtr(), subMaskFrame->getStep());

        // Copy: because I'm going to modify inputImg to show some things on screen
        Mat copyImg = inputImg.clone();

        // Compute Upper and Lower Masks
        Mat upper_mask, lower_mask;
        computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask, mask); // This ones modifies inputImg

        // Sample Mask
        Mat upper_sampled_mask = randomSampling<uchar>(upper_mask, 200, upper_mask);
        Mat lower_sampled_mask = randomSampling<uchar>(lower_mask, 200, lower_mask);

        // Denoise Image
        denoiseImage(copyImg, copyImg);

        // Convert from RGB color space to CIElab
        // CIElab 8 bits image ranges: all channels 0-255.
        Mat imgLab;
        cv::cvtColor(copyImg, imgLab, cv::COLOR_RGB2Lab);

        // Split image in L*, a* and b* channels
        vector<Mat> lab_planes;
        split(imgLab, lab_planes);

        // Create 2D image with only a and b channels
        vector<Mat> ab_planes;
        Mat imgAb;
        ab_planes.push_back(lab_planes[1]); // a channel
        ab_planes.push_back(lab_planes[2]); // b channel
        merge(ab_planes, imgAb);

        // Compute the histogram for the upper (torso) and lower (leggs) parts
        auto u_hist = Histogram2D<uchar>::create(imgAb, upper_sampled_mask);
        auto l_hist = Histogram2D<uchar>::create(imgAb, lower_sampled_mask);

        // Show histogram info
        const auto u_max_item = u_hist->maxFreqItem();
        const auto l_max_item = l_hist->maxFreqItem();
        qDebug() << "u.max" << "(" << u_max_item.point[DIM_X] << u_max_item.point[DIM_Y] << ")" << u_max_item.value;
        qDebug() << "l.max" << "(" << l_max_item.point[DIM_X] << l_max_item.point[DIM_Y] << ")" << l_max_item.value;
        //qDebug() << "avg" << u_hist->avgFreq() << "n" << u_hist->numItems();

        // Show it on an image
        int img_size[] = {400, 400};
        float ab_range[] = {0, 255};
        //const float* hist_ranges[] = { ab_range, ab_range };
        Mat hist2DImg;
        create2DCoordImage(*u_hist, hist2DImg, img_size, ab_range, true, Vec3b(255, 0, 0)); // Blue
        create2DCoordImage(*l_hist, hist2DImg, img_size, ab_range, false, Vec3b(0, 0, 255)); // Red

        // Show
        imshow("hist2DImg", hist2DImg);
        imshow("A.channel", lab_planes[1]);
        imshow("B.channel", lab_planes[2]);
        waitKey(1);

    } // End OpenCV code

    return;
}

// Approach 3: YUV
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
                     CV_8UC3, (void*)subColorFrame->getDataPtr(), subColorFrame->getStep());

        Mat mask(subMaskFrame->getHeight(), subMaskFrame->getWidth(),
                 CV_8UC1, (void*)subMaskFrame->getDataPtr(), subMaskFrame->getStep());

        // Copy: because I'm going to modify inputImg to show some things on screen
        Mat copyImg = inputImg.clone();

        // Compute Upper and Lower Masks
        Mat upper_mask, lower_mask;
        computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask, mask); // This ones modifies inputImg

        // Sample Mask
        Mat upper_sampled_mask = randomSampling<uchar>(upper_mask, 200, upper_mask);
        Mat lower_sampled_mask = randomSampling<uchar>(lower_mask, 200, lower_mask);

        // Denoise Image
        denoiseImage(copyImg, copyImg);

        // Convert from RGB color space to CIElab
        // CIElab 8 bits image ranges: all channels 0-255.
        Mat imgLab;
        cv::cvtColor(copyImg, imgLab, cv::COLOR_RGB2YCrCb);

        // Split image in Y, u and v channels
        vector<Mat> yuv_planes;
        split(imgLab, yuv_planes);

        // Create 2D image with only u and v channels
        vector<Mat> uv_planes;
        Mat imgUv;
        uv_planes.push_back(yuv_planes[1]); // u channel
        uv_planes.push_back(yuv_planes[2]); // v channel
        merge(uv_planes, imgUv);

        // Compute the histogram for the upper (torso) and lower (leggs) parts
        auto u_hist = Histogram2D<uchar>::create(imgUv, upper_sampled_mask);
        auto l_hist = Histogram2D<uchar>::create(imgUv, lower_sampled_mask);

        // Show histogram info
        const auto u_max_item = u_hist->maxFreqItem();
        const auto l_max_item = l_hist->maxFreqItem();
        qDebug() << "u.max" << "(" << u_max_item.point[DIM_X] << u_max_item.point[DIM_Y] << ")" << u_max_item.value;
        qDebug() << "l.max" << "(" << l_max_item.point[DIM_X] << l_max_item.point[DIM_Y] << ")" << l_max_item.value;
        //qDebug() << "avg" << u_hist->avgFreq() << "n" << u_hist->numItems();

        // Show it on an image
        int img_size[] = {400, 400};
        float ab_range[] = {0, 255};
        //const float* hist_ranges[] = { ab_range, ab_range };
        Mat hist2DImg;
        create2DCoordImage(*u_hist, hist2DImg, img_size, ab_range, true, Vec3b(255, 0, 0)); // Blue
        create2DCoordImage(*l_hist, hist2DImg, img_size, ab_range, false, Vec3b(0, 0, 255)); // Red

        // Show
        imshow("hist2DImg", hist2DImg);
        imshow("U.channel", yuv_planes[1]);
        imshow("V.channel", yuv_planes[2]);
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
                     CV_8UC3, (void*)roiColorFrame->getDataPtr(), roiColorFrame->getStep());

        Mat mask(roiMaskFrame->getHeight(), roiMaskFrame->getWidth(),
                 CV_8UC1, (void*)roiMaskFrame->getDataPtr(), roiMaskFrame->getStep());

        // Denoise Image
        denoiseImage(inputImg, inputImg);

        // Discretise Image
        discretiseRGBImage(inputImg, inputImg); // TEST: in YUV Space not RGB

        // Copy: because I'm going to modify inputImg to show some things on screen
        Mat copyImg = inputImg.clone();

        // Compute Upper and Lower Masks
        Mat upper_mask, lower_mask;
        computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask, mask); // This ones modifies inputImg

        // Sample Mask
        Mat upper_sampled_mask = upper_mask; //randomSampling<uchar>(upper_mask, 1500, upper_mask);
        Mat lower_sampled_mask = lower_mask; //randomSampling<uchar>(lower_mask, 1500, lower_mask);

        // Convert to another color space
        cv::cvtColor(copyImg, copyImg, cv::COLOR_RGB2YCrCb); // YUV

        // Compute the histogram for the upper (torso) and lower (leggs) parts
        auto u_hist = Histogram3D<uchar>::create(copyImg, upper_sampled_mask);
        auto l_hist = Histogram3D<uchar>::create(copyImg, lower_sampled_mask);

        /*double sum_head = 0;
        double sum_tail = 0;
        int i=0;

        foreach (auto item, u_hist->items()) {
            if (i < 32) {
                sum_head += item->dist;
            } else {
                sum_tail += item->dist;
            }
            ++i;
        }*/

        //qDebug() << "u.sum32" << sum_head << "u.rest" << sum_tail << "check" << (sum_head + sum_tail);

        /*sum_head = 0;
        sum_tail = 0;
        i = 0;

        foreach (auto item, l_hist->items()) {
            if (i < 32) {
                sum_head += item->dist;
            } else {
                sum_tail += item->dist;
            }
            ++i;
        }*/

        //qDebug() << "l.sum32" << sum_head << "l.rest" << sum_tail << "check" << (sum_head + sum_tail);

        // Show Histogram Info
        //const HistItem3D u_max_item = u_hist->maxFreqItem();
        //const HistItem3D l_max_item = l_hist->maxFreqItem();
        //qDebug() << "u.max" << "(" << u_max_item.point[DIM_X] << u_max_item.point[DIM_Y] << u_max_item.point[DIM_Z] << ")" << u_max_item.value;
        //qDebug() << "l.max" << "(" << l_max_item.point[DIM_X] << l_max_item.point[DIM_Y] << l_max_item.point[DIM_Z] << ")" << l_max_item.value;
        qDebug() << "u.colors" << u_hist->numItems() << "u.maxFreq" << u_hist->maxFreq() << "u.minFreq" << u_hist->minFreq() << "u.avgFreq" << u_hist->avgFreq();
        qDebug() << "l.colors" << l_hist->numItems() << "l.maxFreq" << l_hist->maxFreq() << "l.minFreq" << l_hist->minFreq() << "l.avgFreq" << l_hist->avgFreq();

        // Show it on an image
        Mat colorPalette;
        create2DColorPalette<uchar>(*u_hist, *l_hist, colorPalette);
        cv::cvtColor(colorPalette, colorPalette, cv::COLOR_YCrCb2BGR); // YUV to BGR

        // Show
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
    static cv::Mat color_frame_vector[buffer_size];
    static cv::Mat mask_frame_vector[buffer_size];

    // Start OpenCV code
    {using namespace cv;

        // Use cv::Mat for my color frame and mask frame
        Mat inputImg(roiColorFrame->getHeight(), roiColorFrame->getWidth(),
                     CV_8UC3, (void*)roiColorFrame->getDataPtr(), roiColorFrame->getStep());

        Mat mask(roiMaskFrame->getHeight(), roiMaskFrame->getWidth(),
                 CV_8UC1, (void*)roiMaskFrame->getDataPtr(), roiMaskFrame->getStep());

        //
        // Image Pre-Processing
        //
        Mat img = inputImg.clone();

        // Denoise Image
        denoiseImage(img, img);

        // Discretise Image
        discretiseRGBImage(img, img); // TEST: in YUV Space not RGB

        // Convert to another color space
        cv::cvtColor(img, img, cv::COLOR_RGB2YCrCb); // YUV

        //
        // Buffering
        //
        int vector_idx = frame_counter % buffer_size;
        color_frame_vector[vector_idx] = img;
        mask_frame_vector[vector_idx] = mask.clone();

        // Start to do things when I fill up the buffer
        if (frame_counter >= buffer_size-1)
        {
            // Compute single histogram for the last frame received for comparison purposes
            Mat upper_mask, lower_mask;
            computeUpperAndLowerMasks(color_frame_vector[vector_idx], upper_mask, lower_mask, mask);
            auto u_hist = Histogram3D<uchar>::create(color_frame_vector[vector_idx], upper_mask);
            auto l_hist = Histogram3D<uchar>::create(color_frame_vector[vector_idx], lower_mask);

            Histogram3D<uchar> u_hist_acc, l_hist_acc;

            for (int i=0; i<buffer_size; ++i)
            {
                // Compute Upper and Lower Masks
                Mat upper_mask, lower_mask;
                computeUpperAndLowerMasks(color_frame_vector[i], upper_mask, lower_mask, mask_frame_vector[i]);

                // Sample Mask
                Mat upper_sampled_mask = upper_mask; //randomSampling<uchar>(upper_mask, 1500, upper_mask);
                Mat lower_sampled_mask = lower_mask; //randomSampling<uchar>(lower_mask, 1500, lower_mask);

                // Compute the histogram for the upper (torso) and lower (leggs) parts of each frame in the buffer
                auto u_hist_t = Histogram3D<uchar>::create(color_frame_vector[i], upper_sampled_mask);
                auto l_hist_t = Histogram3D<uchar>::create(color_frame_vector[i], lower_sampled_mask);

                // Accumulate it
                u_hist_acc += *u_hist_t;
                l_hist_acc += *l_hist_t;
            }

            // Show Histogram Info
            auto u_items = u_hist_acc.sortedItems(32);
            auto l_items = l_hist_acc.sortedItems(32);
            int i = 0;

            foreach (auto item, u_items) {
                qDebug() << "u.ac.item" << i++ << "(" << item->point[DIM_X] << item->point[DIM_Y] << item->point[DIM_Z] << ")" << item->value;
            }

            i = 0;
            foreach (auto item, l_items) {
                qDebug() << "l.ac.item" << i++ << "(" << item->point[DIM_X] << item->point[DIM_Y] << item->point[DIM_Z] << ")" << item->value;
            }

            //const HistItem3D u_max_item = u_hist->maxFreqItem();
            //const HistItem3D l_max_item = l_hist->maxFreqItem();
            //qDebug() << "u.max" << "(" << u_max_item.point[DIM_X] << u_max_item.point[DIM_Y] << u_max_item.point[DIM_Z] << ")" << u_max_item.value;
            //qDebug() << "l.max" << "(" << l_max_item.point[DIM_X] << l_max_item.point[DIM_Y] << l_max_item.point[DIM_Z] << ")" << l_max_item.value;
            //qDebug() << "u.colors" << u_hist.numItems() << "u.maxFreq" << u_hist.maxFreq() << "u.minFreq" << u_hist.minFreq() << "u.avgFreq" << u_hist.avgFreq();
            //qDebug() << "l.colors" << l_hist.numItems() << "l.maxFreq" << l_hist.maxFreq() << "l.minFreq" << l_hist.minFreq() << "l.avgFreq" << l_hist.avgFreq();

            // Show it on an image
            Mat colorPalette_acc, colorPalette;
            create2DColorPalette<uchar>(u_hist_acc, l_hist_acc, colorPalette_acc);
            create2DColorPalette<uchar>(*u_hist, *l_hist, colorPalette);
            cv::cvtColor(colorPalette_acc, colorPalette_acc, cv::COLOR_YCrCb2BGR); // YUV to BGR
            cv::cvtColor(colorPalette, colorPalette, cv::COLOR_YCrCb2BGR); // YUV to BGR

            // Show
            imshow("Palette AC", colorPalette_acc);
            imshow("Palette", colorPalette);
            waitKey(1);
        }

    } // End OpenCV code

    frame_counter++;
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
    approach5();

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

cv::Mat PrivacyFilter::convertRGB2Log2D(cv::Mat inputImg)
{
    using namespace cv;

    Mat outputImg = Mat::zeros(inputImg.rows, inputImg.cols, CV_32FC2);

    for (int i=0; i<inputImg.rows; ++i)
    {
        Vec3b* inPixel = inputImg.ptr<Vec3b>(i);
        Vec2f* outPixel = outputImg.ptr<Vec2f>(i);

        for (int j=0; j<inputImg.cols; ++j) {
            // First approach
            /*float divisor = inPixel[j][1] > 0 ? inPixel[j][1] : 1;
            float dividend0 = inPixel[j][0] > 0 ? inPixel[j][0] : 1;
            float dividend1 = inPixel[j][2] > 0 ? inPixel[j][2] : 1;
            outPixel[j][0] = log(dividend0 / divisor);
            outPixel[j][1] = log(dividend1 / divisor);*/

            // Second Approach
            /*if (inPixel[j][1] > 0) {
                float factor0 = float(inPixel[j][0]) / float(inPixel[j][1]);
                float factor1 = float(inPixel[j][2]) / float(inPixel[j][1]);
                outPixel[j][0] = factor0 > 0 ? log(factor0) : 0;
                outPixel[j][1] = factor1 > 0 ? log(factor1) : 0;
            }*/

            // Third approach: Consider RGB colors from 1 to 256
            // min color: log(1/256) ~ -5.6
            // max color: log(256/1) ~ 5.6
            outPixel[j][0] = log( float(inPixel[j][0]+1) / float(inPixel[j][1]+1) ); // log ( R/G )
            outPixel[j][1] = log( float(inPixel[j][2]+1) / float(inPixel[j][1]+1) ); // log ( B/G )
        }
    }

    return outputImg;
}

template <class T>
cv::Mat PrivacyFilter::randomSampling(cv::Mat inputImg, int n, cv::Mat mask)
{
    Q_ASSERT( (mask.rows == 0 && mask.cols == 0) || (mask.rows == inputImg.rows && mask.cols == inputImg.cols) );

    using namespace cv;

    Mat sampledImage = Mat::zeros(inputImg.rows, inputImg.cols, inputImg.type());
    bool useMask = mask.rows > 0 && mask.cols > 0;
    int n_mask_pixels = 0;
    QSet<int> used_samples;
    int k = inputImg.rows * inputImg.cols - 1;

    boost::mt19937 generator;
    generator.seed(time(0));
    boost::uniform_int<> uniform_dist(0, k);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<>> uniform_rnd(generator, uniform_dist);

    if (useMask) {
        for (int i=0; i<mask.rows; ++i) {
            uchar* maskPixel = mask.ptr<uchar>(i);
            for (int j=0; j<mask.cols; ++j) {
                if (maskPixel[j] > 0)
                    n_mask_pixels++;
            }
        }

        n = dai::min<int>(n, n_mask_pixels);
    }

    int i = 0;
    int attempts = 0;

    while (i < n && n < k)
    {
        int z = uniform_rnd();

        if (!used_samples.contains(z)) {

            int row = z / inputImg.cols;
            int col = z % inputImg.cols;

            if (useMask && mask.at<uchar>(row,col) <= 0) {
                attempts++;
                continue;
            }

            sampledImage.at<T>(row,col) = inputImg.at<T>(row,col);
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

void PrivacyFilter::computeUpperAndLowerMasks(const cv::Mat input_img, cv::Mat& upper_mask, cv::Mat& lower_mask, const cv::Mat mask) const
{
    Q_ASSERT( (mask.rows == 0 && mask.cols == 0) || (mask.rows == input_img.rows && mask.cols == input_img.cols) );

    using namespace cv;

    bool useMask = mask.rows > 0 && mask.cols > 0;
    upper_mask = Mat::zeros(input_img.rows, input_img.cols, CV_8UC1);
    lower_mask = Mat::zeros(input_img.rows, input_img.cols, CV_8UC1);

    // For My Capture
    float margins[][2] = {
        0.24f, 0.17f,
        0.42f, 0.75f,
        0.52f, 0.17f,
        0.70f, 0.75f
    };
    /*float margins[][2] = {
        0.0f, 0.17f,
        0.5f, 0.75f,
        0.5f, 0.17f,
        1.0f, 0.75f
    };*/

    // For HuDaAct
    /*float margins[][2] = {
        0.21, 0.18,
        0.52, 0.85,
        0.55, 0.15,
        0.90, 0.85
    };*/

    for (int i=0; i<input_img.rows; ++i)
    {
        //Vec3b* pixel = const_cast<Vec3b*>(input_img.ptr<Vec3b>(i));
        uchar* uMask = upper_mask.ptr<uchar>(i);
        uchar* lMask = lower_mask.ptr<uchar>(i);
        const uchar* maskPixel = useMask ? mask.ptr<uchar>(i) : nullptr;

        for (int j=0; j<input_img.cols; ++j)
        {
            if (useMask && maskPixel[j] <= 0)
                continue;

            if (i >= input_img.rows * margins[0][0] && i < input_img.rows * margins[1][0]) {
                if (j > input_img.cols * margins[0][1] && j < input_img.cols * margins[1][1]) {
                    uMask[j] = 1;
                    //pixel[j][0] = 255;
                }
            } else if (i >= input_img.rows * margins[2][0] && i < input_img.rows * margins[3][0]) {
                if (j > input_img.cols * margins[2][1] && j < input_img.cols * margins[3][1]) {
                    lMask[j] = 1;
                    //pixel[j][2] = 255;
                }
            }
        }
    }
}

template <class T>
void PrivacyFilter::create2DColorPalette(const Histogram3D<T>& upper_hist, const Histogram3D<T>& lower_hist, cv::Mat& output_img)
{
    using namespace cv;

    int width = 400;
    int height = 400;
    int color_width = width / 8;
    int color_height = height / 8;
    int row = 0, col = 0;
    output_img = Mat::zeros(height, width, CV_8UC3);

    QList<const HistItem3D<T>*> upper_list = upper_hist.sortedItems(32);
    QList<const HistItem3D<T>*> lower_list = lower_hist.sortedItems(32);

    for (int i=0; i<400; ++i)
    {
        Vec3b* pixel = output_img.ptr<Vec3b>(i);
        row = i / color_height;

        for (int j=0; j<400; ++j)
        {
            col = j / color_width;

            if (row < 4) {
                int offset = row * 8 + col;
                if (offset < upper_list.size()) {
                    const HistItem3D<T>* item = upper_list.at(offset);
                    pixel[j][0] = item->point[0]; // Y Blue
                    pixel[j][1] = item->point[1]; // u Green
                    pixel[j][2] = item->point[2]; // v Red
                }
            } else {
                int offset = (row * 8 + col) - 32;
                if (offset < lower_list.size()) {
                    const HistItem3D<T>* item = lower_list.at(offset);
                    pixel[j][0] = item->point[0]; // Y Blue
                    pixel[j][1] = item->point[1]; // u Green
                    pixel[j][2] = item->point[2]; // v Red
                }
            }
        }
    }
}

template <class T>
void PrivacyFilter::create2DCoordImage(const Histogram2D<T> &histogram, cv::Mat& output_img, int size[], float input_range[], bool init_output, cv::Vec3b color) const
{
    Q_ASSERT( init_output == true || (output_img.rows == size[0] && output_img.cols == size[1]) );

    using namespace cv;

    if (init_output) {
        //output_img = Mat::zeros(size[0], size[1], CV_8UC3);
        output_img = Mat(size[0], size[1], CV_8UC3, Scalar(0,0,0));
    }

    foreach (auto item, histogram.items())
    {
        float coord_y = dai::normalise<T>(item->point[DIM_Y], input_range[0], input_range[1], 0, size[0]);
        float coord_x = dai::normalise<T>(item->point[DIM_X], input_range[0], input_range[1], 0, size[1]);
        float factor = 1.0f; // dai::normalise<int>(item->value, histogram.minFreq(), histogram.maxFreq(), 0.98f, 1.0f);
        color[0] = float(color[0]) * factor;
        color[1] = float(color[1]) * factor;
        color[2] = float(color[2]) * factor;
        output_img.at<Vec3b>(coord_y, coord_x) = color;
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
                    CV_8UC3, (void*)colorFrame->getDataPtr(), colorFrame->getStep());
    Mat mask;

    // Set Mask if it exists
    if (maskFrame) {
        mask = Mat(maskFrame->getHeight(), maskFrame->getWidth(), CV_8UC1, (void*)maskFrame->getDataPtr(), maskFrame->getStep());
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

/*void PrivacyFilter::blurEffect(shared_ptr<DataFrame> frame)
{
    ColorFrame* colorFrame = (ColorFrame*) frame.get();
    ColorFrame  background = *colorFrame;
    cv::Mat newImag(480, 640, CV_8UC3,  const_cast<RGBColor*>(colorFrame->getDataPtr()));

    cv::GaussianBlur( newImag, newImag, cv::Size( 29, 29 ), 0, 0 );

    for (int i=0; i<480; ++i) {
        for (int j=0; j<640; ++j) {
            if (m_userMask->getItem(i, j) == 0) {
                RGBColor color = background.getItem(i, j);
                colorFrame->setItem(i, j, color);
            }
        }
    }
}

void PrivacyFilter::invisibilityEffect(shared_ptr<DataFrame> frame)
{
    if (m_userMask == nullptr)
        return;

    ColorFrame* colorFrame = (ColorFrame*) frame.get();

    // Get initial background at 20th frame
    if (colorFrame->getIndex() == 20) {
        m_background = *colorFrame;
    }

    // Update Background and make replacement
    for (int i=0; i<m_userMask->getHeight(); ++i)
    {
        for (int j=0; j<m_userMask->getWidth(); ++j)
        {
            uint8_t uLabel = m_userMask->getItem(i, j);

            if (uLabel != 0) {
                if (m_enabled) {
                    RGBColor bgColor = m_background.getItem(i, j);
                    colorFrame->setItem(i, j, bgColor);
                }
            }
            else {
                RGBColor color = colorFrame->getItem(i, j);
                m_background.setItem(i, j, color);
            }
        }
    }
}*/

} // End Namespace
