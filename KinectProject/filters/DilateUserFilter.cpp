#include "DilateUserFilter.h"
#include <opencv2/opencv.hpp>
#include "types/UserFrame.h"

namespace dai {

void DilateUserFilter::applyFilter(shared_ptr<DataFrame> frame)
{
    if (!m_enabled)
        return;

    UserFrame* userFrame = (UserFrame*) frame.get();

    // User mask have to cover user in color frame completely
    dilateUserMask(const_cast<uint8_t*>(userFrame->getDataPtr()));
}

void DilateUserFilter::dilateUserMask(uint8_t *labels)
{
    cv::Mat newImag(480, 640, cv::DataType<uint8_t>::type, labels);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS,
                                               cv::Size(2*m_dilation_size + 1, 2*m_dilation_size+1),
                                               cv::Point( m_dilation_size, m_dilation_size ) );
    cv::dilate(newImag, newImag, kernel);
}

void DilateUserFilter::setDilationSize(int value)
{
    m_dilation_size = value;
}

} // End Namespace
