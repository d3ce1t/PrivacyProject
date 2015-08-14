#include "opencv_utils.h"

namespace dai {

void colorImageWithMask(cv::Mat input_img, cv::Mat output_img, cv::Mat upper_mask, cv::Mat lower_mask)
{
    Q_ASSERT(input_img.rows == output_img.rows && input_img.cols == output_img.cols);

    using namespace cv;

    if (input_img.data != output_img.data) {
        output_img = input_img.clone();
    }

    for (int i=0; i<output_img.rows; ++i)
    {
        Vec3b* out_pixel = output_img.ptr<Vec3b>(i);
        uchar* um_pixel = upper_mask.ptr<uchar>(i);
        uchar* lm_pixel = lower_mask.ptr<uchar>(i);

        for (int j=0; j<output_img.cols; ++j)
        {
            if (um_pixel[j] > 0) {
                out_pixel[j][0] = 255; // R
            } else if (lm_pixel[j] > 0) {
                out_pixel[j][2] = 255;
            }
        }
    }
}

cv::Mat computeIntegralImage(cv::Mat image)
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

// Count number of pixels of the silhouette / number of pixels of the bounding box
double computeOccupancy(shared_ptr<MaskFrame> mask, int *outNumPixels)
{
    double occupancy;
    int n_pixels = 0;

    for (int i=0; i<mask->height(); ++i) {
        const uint8_t* pixelMask =  mask->getRowPtr(i);
        for (int j=0; j<mask->width(); ++j) {
            if (pixelMask[j] > 0 && pixelMask[j] < 255) {
                n_pixels++;
            }
        }
    }

    occupancy = (float) n_pixels / (float) (mask->height() * mask->width());

    if (outNumPixels) {
        *outNumPixels = n_pixels;
    }

    return occupancy;
}

void computeUpperAndLowerMasks(const cv::Mat& input_img, cv::Mat& upper_mask, cv::Mat& lower_mask, const cv::Mat mask)
{
    Q_ASSERT( (mask.rows == 0 && mask.cols == 0) || (mask.rows == input_img.rows && mask.cols == input_img.cols) );

    using namespace cv;

    bool useMask = mask.rows > 0 && mask.cols > 0;
    upper_mask = Mat::zeros(input_img.rows, input_img.cols, CV_8UC1);
    lower_mask = Mat::zeros(input_img.rows, input_img.cols, CV_8UC1);

    // For My Capture
    /*float margins[][2] = {
        0.24f, 0.17f,
        0.42f, 0.75f,
        0.52f, 0.17f,
        0.70f, 0.75f
    };*/

    // For CAVIAR4REID
    float margins[][2] = {
        0.12f, 0.15f,
        0.52f, 0.85f,
        0.55f, 0.15f,
        0.95f, 0.85f
    };

    for (int i=0; i<input_img.rows; ++i)
    {
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
                }
            } else if (i >= input_img.rows * margins[2][0] && i < input_img.rows * margins[3][0]) {
                if (j > input_img.cols * margins[2][1] && j < input_img.cols * margins[3][1]) {
                    lMask[j] = 1;
                }
            }
        }
    }
}

void convertIndexed8842RGB(const cv::Mat& indexed_img, cv::Mat& output_img)
{
    Q_ASSERT(output_img.data == nullptr ||
             (output_img.rows == indexed_img.rows && output_img.cols == indexed_img.cols &&
              output_img.channels() == 3 && output_img.type() == CV_8U) );

    using namespace cv;

    if (output_img.data == nullptr)
        output_img = cv::Mat::zeros(indexed_img.rows, indexed_img.cols, CV_8UC3);

    for (int i=0; i<indexed_img.rows; ++i)
    {
        const uchar* in_pixel = indexed_img.ptr<uchar>(i);
        Vec3b* out_pixel = output_img.ptr<Vec3b>(i);

        for (int j=0; j<indexed_img.cols; ++j)
        {
            uchar level_red = in_pixel[j] >> 5;
            uchar level_green = (in_pixel[j] ^ 0xE3) >> 2;
            uchar level_blue = in_pixel[j] ^ 0xFC;
            out_pixel[j][0] = level_red * 32;
            out_pixel[j][1] = level_green * 32;
            out_pixel[j][2] = level_blue * 64;
        }
    }
}

void convertIndexed1616162RGB(const cv::Mat& indexed_img, cv::Mat& output_img)
{
    Q_ASSERT(output_img.data == nullptr ||
             (output_img.rows == indexed_img.rows && output_img.cols == indexed_img.cols &&
              output_img.channels() == 3 && output_img.type() == CV_8U) );

    using namespace cv;

    if (output_img.data == nullptr)
        output_img = cv::Mat::zeros(indexed_img.rows, indexed_img.cols, CV_8UC3);

    for (int i=0; i<indexed_img.rows; ++i)
    {
        const ushort* in_pixel = indexed_img.ptr<ushort>(i);
        Vec3b* out_pixel = output_img.ptr<Vec3b>(i);

        for (int j=0; j<indexed_img.cols; ++j)
        {
            uchar level_red = in_pixel[j] >> 8;
            uchar level_green = (in_pixel[j] ^ 0xF0F) >> 4;
            uchar level_blue = in_pixel[j] ^ 0xFF0;
            out_pixel[j][0] = level_red * 16;
            out_pixel[j][1] = level_green * 16;
            out_pixel[j][2] = level_blue * 16;
        }
    }
}

// Convert a RGB image to 8-8-4 levels RGB
cv::Mat convertRGB2Indexed884(const cv::Mat& inputImg)
{
    using namespace cv;

    Mat outputImg = cv::Mat::zeros(inputImg.rows, inputImg.cols, CV_8UC1);

    for (int i=0; i<inputImg.rows; ++i)
    {
        const Vec3b* in_pixel = inputImg.ptr<Vec3b>(i);
        uchar* out_pixel = outputImg.ptr<uchar>(i);

        for (int j=0; j<inputImg.cols; ++j)
        {
            uchar level_red = in_pixel[j][0] / 32; // 256 / 8
            uchar level_green = in_pixel[j][1] / 32; // 256 / 8
            uchar level_blue = in_pixel[j][2] / 64; // 256 / 4
            out_pixel[j] = 32 * level_red + 4 * level_green + level_blue;
        }
    }

    return outputImg;
}

// Convert a RGB image to 16-16-16 levels RGB
cv::Mat convertRGB2Indexed161616(const cv::Mat& inputImg)
{
    using namespace cv;

    Mat outputImg = cv::Mat::zeros(inputImg.rows, inputImg.cols, CV_16UC1);

    for (int i=0; i<inputImg.rows; ++i)
    {
        const Vec3b* in_pixel = inputImg.ptr<Vec3b>(i);
        ushort* out_pixel = outputImg.ptr<ushort>(i);

        for (int j=0; j<inputImg.cols; ++j)
        {
            uchar level_red = in_pixel[j][0] / 16; // 256 / 16
            uchar level_green = in_pixel[j][1] / 16; // 256 / 16
            uchar level_blue = in_pixel[j][2] / 16; // 256 / 16
            out_pixel[j] = 256 * level_red + 16 * level_green + level_blue;
        }
    }

    return outputImg;
}

cv::Mat convertRGB2Log2DAsMat(const cv::Mat &inputImg)
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

QList<Point2f> convertRGB2Log2DAsList(const QList<Point3b>& list)
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

void denoiseImage(cv::Mat input_img, cv::Mat output_img)
{
    // without filter: u.min 7 u.max 4466 u.nz 505 l.min 0 l.max 1473 l.nz 331
    // filter:         u.min 7 u.max 3976 u.nz 577 l.min 0 l.max 1490 l.nz 323
    medianBlur(input_img, output_img, 7);
    //GaussianBlur(input_img, output_img, cv::Size(7, 7), 0, 0);
    //GaussianBlur(output_img, output_img, cv::Size(7, 7), 0, 0);
    //bilateralFilter(image, denoiseImage, 9, 18, 4.5f);
}

void discretiseRGBImage(cv::Mat input_img, cv::Mat output_img)
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

/**
 * Process each pixel of the input image using a filter function and obtaint as a result
 * an output image. The image used as input is not modified.
 *
 * @brief filterMask
 * @param input
 * @param output
 * @param filter
 */
void filterMask(cv::Mat input, cv::Mat& output, std::function<void (uchar in, uchar& out)> filter)
{
    Q_ASSERT(output.data == nullptr || (input.rows == output.rows && input.cols == output.cols) );

    if (output.data == nullptr) {
        output = cv::Mat::zeros(input.rows, input.cols, CV_8UC1);
    }

    for (int i=0; i<input.rows; ++i)
    {
        uchar* input_pixel = input.ptr<uchar>(i);
        uchar* output_pixel = output.ptr<uchar>(i);

        for (int j=0; j<input.cols; ++j)
        {
            filter(input_pixel[j], output_pixel[j]);
        }
    }
}

void create2DCoordImage(const QList<QList<Point2f>*> &input_list, const QList<cv::Vec3b>& color_list,
                                       cv::Mat& output_img, float input_range[])
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

cv::Mat createMask(cv::Mat input_img, int min_value, int* nonzero_counter, bool filter)
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

} // End Namespace
