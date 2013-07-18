#include "DilateUserFilter.h"
#include <opencv2/opencv.hpp>
#include "types/UserFrame.h"

namespace dai {

void DilateUserFilter::applyFilter(shared_ptr<DataFrame> frame)
{
    UserFrame* userFrame = (UserFrame*) frame.get();

    // User mask have to cover user in color frame completely
    dilateUserMask(const_cast<u_int8_t*>(userFrame->getDataPtr()));
}

void DilateUserFilter::dilateUserMask(u_int8_t *labels)
{
    cv::Mat newImag(480, 640, cv::DataType<u_int8_t>::type, labels);
    int dilation_size = 16;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS,
                                               cv::Size(2*dilation_size + 1, 2*dilation_size+1),
                                               cv::Point( dilation_size, dilation_size ) );
    cv::dilate(newImag, newImag, kernel);
}

} // End Namespace
