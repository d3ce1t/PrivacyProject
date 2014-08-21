#ifndef OPENCV_UTILS_H
#define OPENCV_UTILS_H

#include <opencv2/opencv.hpp>
#include <QList>
#include "types/Histogram.h"

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

void computeUpperAndLowerMasks(const cv::Mat& input_img, cv::Mat& upper_mask, cv::Mat& lower_mask, const cv::Mat mask = cv::Mat())
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

template <class T, int N>
void createHistDistImage(const QList<Histogram<T,N>*>& hist_list, const QList<cv::Scalar> &color_list, cv::Mat& output_img)
{
    Q_ASSERT(hist_list.size() == color_list.size());

    using namespace cv;

    const int width = 512;
    const int height = 400;

    // Init output image
    output_img = Mat::zeros(height, width, CV_8UC3);

    // Search min and max freq
    float maxFreq = 0.0f, minFreq = 1.0f;
    int maxRange = 0, minRange = 999999;

    for (auto it = hist_list.constBegin(); it != hist_list.constEnd(); ++it) {
        const HistBin<T,N>& max_item = (*it)->maxFreqItem();
        const HistBin<T,N>& min_item = (*it)->minFreqItem();
        maxFreq = dai::max<float>(max_item.dist, maxFreq);
        minFreq = dai::min<float>(min_item.dist, minFreq);
        maxRange = dai::max<int>((*it)->maxRange(), maxRange);
        minRange = dai::min<int>((*it)->minRange(), minRange);
    }

    // Draw all distributions
    const int space_size = std::pow(maxRange - minRange + 1, N);
    int color_idx = 0;

    for (auto it_hist = hist_list.constBegin(); it_hist != hist_list.constEnd(); ++it_hist)
    {
        auto items = (*it_hist)->bins();

        float bin_w = double(width)/ (space_size + 2);

        for (auto it_items = items.constBegin(); it_items != items.constEnd(); ++it_items)
        {
            float curr_value = dai::normalise<float>( (*it_items)->dist, 0, maxFreq, height-3, 0);
            int curr_key = (*it_items)->key + 1;

            line(output_img,
                 cv::Point( cvRound( bin_w*curr_key ), height),
                 cv::Point( cvRound( bin_w*curr_key ), cvRound(curr_value) ),
                 color_list.at(color_idx), 1, 8, 0 );
        }

        ++color_idx;
    }
}

} // End Namespace

#endif // OPENCV_UTILS_H
