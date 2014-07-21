#ifndef PRIVACYFILTER_H
#define PRIVACYFILTER_H

#include "playback/FrameListener.h"
#include "playback/FrameGenerator.h"
#include "ogre/OgreScene.h"
#include "viewer/Scene2DPainter.h"
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>
#include <QOpenGLContext>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <QImage>
#include <QFile>

namespace dai {

class PrivacyFilter : public FrameListener, public FrameGenerator
{
public:
    PrivacyFilter();
    ~PrivacyFilter();
    void initialise();
    void newFrames(const QHashDataFrames dataFrames) override;
    void enableFilter(ColorFilter filterType);

protected:
    void afterStop() override;
    QHashDataFrames produceFrames() override;
    void freeResources();

private:
    void dilateUserMask(uint8_t *labels);
    cv::Mat calcHistogram(shared_ptr<ColorFrame> colorFrame, shared_ptr<MaskFrame> mask);
    std::vector<cv::Rect> faceDetection(cv::Mat frameGray, bool equalised = false);
    cv::Mat computeIntegralImage(cv::Mat image);
    template <class T>
    cv::Mat interleaveMatChannels(cv::Mat inputMat, cv::Mat mask = cv::Mat(), int type = CV_32SC1);
    double computeOccupancy(shared_ptr<MaskFrame> mask, int *outNumPixels = nullptr);

    QHashDataFrames m_frames;
    QOpenGLContext* m_glContext;
    QOpenGLFunctions* m_gles;
    QOffscreenSurface m_surface;
    bool m_initialised;
    Scene2DPainter* m_scene;
    OgreScene* m_ogreScene;
    QOpenGLFramebufferObject* m_fboDisplay;
    ColorFilter m_filter;
    cv::Mat newSpace;
    QImage myImage;
    QFile m_file;
    QTextStream m_out;
    //cv::CascadeClassifier m_face_cascade;
};

} // End Namespace

#endif // PRIVACYFILTER_H
