#include "PrivacyFilter.h"
#include "types/MaskFrame.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "viewer/SilhouetteItem.h"
#include "viewer/SkeletonItem.h"
#include "types/MetadataFrame.h"
#include <QDebug>
#include <QThread>
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

// Test Whatever thing
void PrivacyFilter::test1()
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
    shared_ptr<ColorFrame> subColorFrame = colorFrame->subFrame(bb.getMin().y(),bb.getMin().x(),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> subMaskFrame = maskFrame->subFrame(bb.getMin().y(),bb.getMin().x(),
                                                             bb.size().width(), bb.size().height());

    /*cv::Mat subImage(subColorFrame->getHeight(), subColorFrame->getWidth(),
                     CV_8UC3, (void*)subColorFrame->getDataPtr(), subColorFrame->getStep());
    cv::Mat greyImage;

    cv::cvtColor(subImage, greyImage, cv::COLOR_RGB2GRAY);

    // Face Detection
    std::vector<cv::Rect> faces = faceDetection(greyImage);

        for (size_t i=0; i<faces.size(); ++i) {
            BoundingBox faceBB(Point3f(bb.getMin().x() + faces[i].x, bb.getMin().y() + faces[i].y, 0),
                               Point3f(bb.getMin().x() + faces[i].x + faces[i].width,
                                       bb.getMin().y() + faces[i].y + faces[i].height, 0) );

            metadataFrame->boundingBoxes().append(faceBB);
        }*/

    // Compute Histogram
    calcHistogram(subColorFrame, subMaskFrame);

}

// Test 2D log color space
// Paper: Color Invariants for Person Reidentification
void PrivacyFilter::test2()
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
    shared_ptr<ColorFrame> subColorFrame = colorFrame->subFrame(bb.getMin().y(),bb.getMin().x(),
                                                                bb.size().width(), bb.size().height());
    shared_ptr<MaskFrame> subMaskFrame = maskFrame->subFrame(bb.getMin().y(),bb.getMin().x(),
                                                             bb.size().width(), bb.size().height());

    // Start OpenCV code
    {using namespace cv;

        Mat inputImg(subColorFrame->getHeight(), subColorFrame->getWidth(),
                     CV_8UC3, (void*)subColorFrame->getDataPtr(), subColorFrame->getStep());

        Mat mask(subMaskFrame->getHeight(), subMaskFrame->getWidth(),
                 CV_8UC1, (void*)subMaskFrame->getDataPtr(), subMaskFrame->getStep());

        // Compute Upper and Lower Masks
        Mat upper_mask;
        Mat lower_mask;
        computeUpperAndLowerMasks(inputImg, upper_mask, lower_mask, mask);

        // Denoise Image
        Mat denoise_img;
        GaussianBlur(inputImg, denoise_img, cv::Size(7, 7), 0, 0);
        GaussianBlur(denoise_img, denoise_img, cv::Size(7, 7), 0, 0);
        //bilateralFilter(image, denoiseImage, 9, 18, 4.5f);

        // Sample Image
        Mat upper_sampled_mask = simpleRandomSampling<uchar>(upper_mask, 400, upper_mask);
        Mat lower_sampled_mask = simpleRandomSampling<uchar>(lower_mask, 400, lower_mask);

        // Convert from RGB color space to 2D log color space
        Mat imgLog = convertRGB2Log2D(inputImg);

        //
        // Let's party
        //

        // Compute the histogram
        int img_size[] = {400, 400};
        float log_range[] = {-5.6f, 5.6f};
        const float* hist_ranges[] = { log_range, log_range };
        int channels[] = {0, 1};
        Mat u_hist, l_hist;
        calcHist( &imgLog, 1, channels, upper_sampled_mask, u_hist, 2, img_size, hist_ranges, true, false);
        calcHist( &imgLog, 1, channels, lower_sampled_mask, l_hist, 2, img_size, hist_ranges, true, false);

        // Create masks for the upper and lower histograms
        int u_nonzero_counter, l_nonzero_counter;
        Mat u_hist_mask = createMask(u_hist, &u_nonzero_counter);
        Mat l_hist_mask = createMask(l_hist, &l_nonzero_counter);

        // Get min and max
        double u_min, u_max, l_min, l_max;
        minMaxLoc(u_hist, &u_min, &u_max);
        minMaxLoc(l_hist, &l_min, &l_max);
        qDebug() << "u.min" << u_min << "u.max" << u_max << "u.nz" << u_nonzero_counter <<
                    "l.min" << l_min << "l.max" << l_max << "l.nz" << l_nonzero_counter;

        // Histogram of the histogram (Upper part & Lower part)
        int histSize = dai::max<float>(u_max, l_max);
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

        /*Mat u_hist_norm;
        divide(u_hist, u_max, u_hist_norm);

        Mat l_hist_norm;
        divide(l_hist, l_max, l_hist_norm);

        Mat diff_hist;
        absdiff(u_hist, l_hist, diff_hist);*/

        normalize(u_hist, u_hist, 0,1, NORM_MINMAX, -1, Mat() );
        normalize(l_hist, l_hist, 0,1, NORM_MINMAX, -1, Mat() );

        // Show
        //imshow("Sample.Image", sampled_img);
        //imshow("Denoise.Image", denoise_img);
        imshow("H.image", histImage);
        //imshow("ImgLOG0", log_planes[0]);
        //imshow("ImgLOG1", log_planes[1]);
        //imshow("Hist", hist);
        imshow("U.hist", u_hist);
        imshow("L.hist", l_hist);
        //imshow("D.hist", diff_hist);
        //imshow("NU.hist", u_hist_norm);
        //imshow("NL.hist", l_hist_norm);
        waitKey(1);

    } // End OpenCV code

    return;
}

QHashDataFrames PrivacyFilter::produceFrames()
{
    Q_ASSERT(m_frames.contains(DataFrame::Color) && m_frames.contains(DataFrame::Mask) &&
             m_frames.contains(DataFrame::Skeleton) && m_frames.contains(DataFrame::Metadata));

    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(m_frames.value(DataFrame::Color));
    shared_ptr<MaskFrame> maskFrame = static_pointer_cast<MaskFrame>(m_frames.value(DataFrame::Mask));

    //test1();
    test2();

    // Dilate mask to create a wide border (value = 255)
    shared_ptr<MaskFrame> inputMask = static_pointer_cast<MaskFrame>(m_frames.value(DataFrame::Mask));
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
    }

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

cv::Mat PrivacyFilter::createMask(cv::Mat input_img, int* nonzero_counter) const
{
    using namespace cv;

    Mat output_mask = Mat::zeros(input_img.rows, input_img.cols, CV_8UC1);
    int counter = 0;

    for (int i=0; i<input_img.rows; ++i)
    {
        float* pixel = input_img.ptr<float>(i);

        for (int j=0; j<input_img.cols; ++j)
        {
            if (pixel[j] > 0) {
                output_mask.at<uchar>(i, j) = 1;
                counter++;
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
cv::Mat PrivacyFilter::simpleRandomSampling(cv::Mat inputImg, int n, cv::Mat mask)
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

    for (int i=0; i<input_img.rows; ++i)
    {
        Vec3b* pixel = const_cast<Vec3b*>(input_img.ptr<Vec3b>(i));
        uchar* uMask = upper_mask.ptr<uchar>(i);
        uchar* lMask = lower_mask.ptr<uchar>(i);
        const uchar* maskPixel = useMask ? mask.ptr<uchar>(i) : nullptr;

        for (int j=0; j<input_img.cols; ++j)
        {
            if (useMask && maskPixel[j] <= 0)
                continue;

            if (i > input_img.rows * 0.12 && i < input_img.rows * 0.45) {
                uMask[j] = 1;
                //pixel[j][0] = 255;
            } else if (i > input_img.rows * 0.50 && i < input_img.rows * 0.78) {
                lMask[j] = 1;
                pixel[j][2] = 255;
            }
        }
    }

    return;
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
             Point( bin_w*(i-1), hist_h - cvRound(l_hist.at<float>(i-1)) ) ,
             Point( bin_w*(i), hist_h - cvRound(l_hist.at<float>(i)) ),
             Scalar( 0, 0, 255), 1, 8, 0  );
        line(histImage,
             Point( bin_w*(i-1), hist_h - cvRound(a_hist.at<float>(i-1)) ) ,
             Point( bin_w*(i), hist_h - cvRound(a_hist.at<float>(i)) ),
             Scalar( 0, 255, 0), 1, 8, 0  );
        line(histImage,
             Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
             Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
             Scalar( 255, 0, 0), 1, 8, 0  );
        line(histImage,
             Point( bin_w*(i-1), hist_h - cvRound(int_hist.at<float>(i-1)) ) ,
             Point( bin_w*(i), hist_h - cvRound(int_hist.at<float>(i)) ),
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

                pixel24b |= (pixel_tmp[1] & 0x01) << k+1;
                pixel_tmp[1] >>= 1;

                if (nChannels == 3) {
                    pixel24b |= (pixel_tmp[2] & 0x01) << k+2;
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
