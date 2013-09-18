#include "BlurFilter.h"
#include <opencv2/opencv.hpp>
#include "types/ColorFrame.h"

namespace dai {

void BlurFilter::applyFilter(shared_ptr<DataFrame> frame)
{
    if (!m_enabled || !m_userMask)
        return;

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

} // End Namespace
