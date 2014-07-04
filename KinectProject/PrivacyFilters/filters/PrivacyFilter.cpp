#include "PrivacyFilter.h"
#include "filters/DilateUserFilter.h"
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

} // End Namespace
