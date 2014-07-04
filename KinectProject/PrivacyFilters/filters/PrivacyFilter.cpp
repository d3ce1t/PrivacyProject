#include "PrivacyFilter.h"
#include "types/MaskFrame.h"
#include <opencv2/opencv.hpp>

namespace dai {

PrivacyFilter::PrivacyFilter()
{
}

PrivacyFilter::~PrivacyFilter()
{
   stopListener();
}

// This method is called from a thread from the PlaybackControl
void PrivacyFilter::newFrames(const QHashDataFrames dataFrames)
{
    // Copy frames (1 ms)
    m_frames.clear();

    foreach (DataFrame::FrameType key, dataFrames.keys()) {
        shared_ptr<DataFrame> frame = dataFrames.value(key);
        m_frames.insert(key, frame->clone());
    }

    // Check if the frames has been copied correctly
    if (!hasExpired()) {
        if (!produce()) {
            qDebug() << "PrivacyFilter: Nothing produced";
        }
    }
    else {
        qDebug() << "Frame has expired!";
    }
}

QHashDataFrames PrivacyFilter::produceFrames()
{
    // Dilate mask to create a wide border (value = 255)
    if (m_frames.contains(DataFrame::Mask))
    {
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
    }

    return m_frames;
}

void PrivacyFilter::dilateUserMask(uint8_t *labels)
{
    int dilationSize = 18;
    cv::Mat newImag(480, 640, cv::DataType<uint8_t>::type, labels);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS,
                                               cv::Size(2*dilationSize + 1, 2*dilationSize+1),
                                               cv::Point( dilationSize, dilationSize ) );
    cv::dilate(newImag, newImag, kernel);
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
