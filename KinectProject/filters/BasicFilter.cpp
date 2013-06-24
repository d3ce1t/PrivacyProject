#include "BasicFilter.h"
#include "../types/DepthFrame.h"
#include "opencv2/opencv.hpp"
#include <QDebug>

namespace dai {

BasicFilter::BasicFilter()
    : m_background(640, 480)
{
}

/*void BasicFilter::processFrame(dai::DataFrameList framesList, InstanceViewer* viewer)
{
    Q_UNUSED(viewer);

    dai::ColorFrame* colorFrame = (dai::ColorFrame*) framesList.at(0);
    dai::DepthFrame* depthFrame = (dai::DepthFrame*) framesList.at(1);

    // User mask have to cover user in color frame completely
    dilateUserMask(const_cast<short int*>(depthFrame->getLabelPtr()));

    // Get initial background at 20th frame
    if (colorFrame->getIndex() == 20) {
        m_background = *colorFrame;
    }

    for (int i=0; i<depthFrame->getHeight(); ++i)
    {
        for (int j=0; j<depthFrame->getWidth(); ++j)
        {
            short uLabel = depthFrame->getLabel(i, j);

            if (uLabel != 0) {
                RGBAColor bgColor = m_background.getItem(i, j);
                colorFrame->setItem(i, j, bgColor);
            }
            else {
                RGBAColor color = colorFrame->getItem(i, j);
                m_background.setItem(i, j, color);
            }
        }
    }
}*/
void BasicFilter::dilateUserMask(short int* labels)
{
    cv::Mat newImag(480, 640, cv::DataType<short int>::type, labels);
    int dilation_size = 15;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS,
                                               cv::Size(2*dilation_size + 1, 2*dilation_size+1),
                                               cv::Point( dilation_size, dilation_size ) );
    cv::dilate(newImag, newImag, kernel);
}

} // End namespace
