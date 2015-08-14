#ifndef OPENCV_UTILS_H
#define OPENCV_UTILS_H

#include <opencv2/opencv.hpp>
#include <QList>
#include <QSet>
#include <QDebug>
#include "types/Histogram.h"
#include "types/MaskFrame.h"
#include <functional>
#include <boost/random.hpp>

namespace dai {

void colorImageWithMask(cv::Mat input_img, cv::Mat output_img, cv::Mat upper_mask, cv::Mat lower_mask);

cv::Mat computeIntegralImage(cv::Mat image);

// Count number of pixels of the silhouette / number of pixels of the bounding box
double computeOccupancy(shared_ptr<MaskFrame> mask, int *outNumPixels = nullptr);

void computeUpperAndLowerMasks(const cv::Mat& input_img, cv::Mat& upper_mask, cv::Mat& lower_mask, const cv::Mat mask = cv::Mat());

void convertIndexed8842RGB(const cv::Mat& indexed_img, cv::Mat& output_img);

void convertIndexed1616162RGB(const cv::Mat& indexed_img, cv::Mat& output_img);

// Convert a RGB image to 8-8-4 levels RGB
cv::Mat convertRGB2Indexed884(const cv::Mat& inputImg);

// Convert a RGB image to 16-16-16 levels RGB
cv::Mat convertRGB2Indexed161616(const cv::Mat& inputImg);

cv::Mat convertRGB2Log2DAsMat(const cv::Mat &inputImg);

QList<Point2f> convertRGB2Log2DAsList(const QList<Point3b>& list);

template <class T>
int count_pixels_nz(const cv::Mat& inputImg)
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
void createHistImage(const QList<Histogram<T,N>*>& hist_list, const QList<cv::Scalar> &color_list, cv::Mat& output_img)
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

void denoiseImage(cv::Mat input_img, cv::Mat output_img);

void discretiseRGBImage(cv::Mat input_img, cv::Mat output_img);

template <class T>
void for_each_pixel(cv::Mat input, std::function<void (T& pixel, int row, int column)> func)
{
    for (int i=0; i<input.rows; ++i)
    {
        T* input_pixel = input.ptr<T>(i);

        for (int j=0; j<input.cols; ++j) {
            func(input_pixel[j], i, j);
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
void filterMask(cv::Mat input, cv::Mat& output, std::function<void (uchar in, uchar& out)> filter);

template <class T>
cv::Mat interleaveMatChannels(cv::Mat inputMat, cv::Mat mask = cv::Mat(), int type = CV_32SC1)
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

template <class T, int N>
cv::Mat randomSamplingAsMat(const cv::Mat &inputImg, int n, const cv::Mat &mask = cv::Mat())
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
QList<Point<T,N>> randomSampling(const cv::Mat &inputImg, int n, const cv::Mat &mask = cv::Mat())
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
cv::Mat samplingAsMat(const cv::Mat &inputImg, const cv::Mat &mask = cv::Mat())
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
QList<Point<T,N>> samplingAsList(const cv::Mat& inputImg, const cv::Mat& mask = cv::Mat())
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
void create2DCoordImage(cv::Mat input_img, cv::Mat& output_img, int size[], float input_range[], bool init_output, cv::Vec3b color)
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

template <class T>
void create2DCoordImage(const QList<Histogram2D<T>*>& hist_list, int n_items, const QList<cv::Vec3b> &color_list, cv::Mat& output_img,
                                       float input_range[])
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

void create2DCoordImage(const QList<QList<Point2f>*> &input_list, const QList<cv::Vec3b>& color_list,
                                       cv::Mat& output_img, float input_range[]);

template <class T>
void create2DColorPalette(const QList<const HistBin3D<T>*>& upper_hist, const QList<const HistBin3D<T>*>& lower_hist, cv::Mat& output_img)
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

cv::Mat createMask(cv::Mat input_img, int min_value, int* nonzero_counter, bool filter);

template <typename T, int N>
bool compare(const cv::Mat& inputImg, const QList<Point<T,N>>& point_list, const cv::Mat& mask)
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


} // End Namespace

#endif // OPENCV_UTILS_H
