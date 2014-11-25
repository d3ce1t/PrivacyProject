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
#include "types/Histogram.h"

extern void PrivacyLib_InitResources();

namespace dai {

class PrivacyFilter : public FrameListener, public FrameGenerator
{
    shared_ptr<QHashDataFrames> m_frames;
    QOpenGLContext* m_glContext;
    QOpenGLFunctions* m_gles;
    QOffscreenSurface m_surface;
    bool m_initialised;
    Scene2DPainter* m_scene;
    OgreScene* m_ogreScene;
    QOpenGLFramebufferObject* m_fboDisplay;
    ColorFilter m_filter;
    QFile m_file;
    QTextStream m_out;
    bool m_make_capture;
    cv::CascadeClassifier m_face_cascade;

public:
    PrivacyFilter();
    ~PrivacyFilter();
    void newFrames(const QHashDataFrames dataFrames) override;
    void singleFrame(const QHashDataFrames dataFrames);
    void enableFilter(ColorFilter filterType);
    void captureImage();

protected:
    void initialise();
    void afterStop() override;
    shared_ptr<QHashDataFrames> allocateMemory() override;
    void produceFrames(QHashDataFrames& output) override;
    void freeResources();

private:
    void dilateUserMask(uint8_t *labels);
    std::vector<cv::Rect> faceDetection(shared_ptr<ColorFrame> frame);
    std::vector<cv::Rect> faceDetection(cv::Mat frameGray, bool equalised = false);
};

} // End Namespace

#endif // PRIVACYFILTER_H
