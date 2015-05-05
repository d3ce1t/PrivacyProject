#ifndef PRIVACYFILTER_H
#define PRIVACYFILTER_H

#include "playback/FrameListener.h"
#include "playback/FrameGenerator.h"
#include <QOffscreenSurface>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <QImage>
#include <QFile>
#include "types/ColorFrame.h"
#include "viewer/types.h"

extern void PrivacyLib_InitResources();

class QOpenGLContext;
class QOpenGLFramebufferObject;
class QOpenGLFunctions;
class OgreScene;

namespace dai {

class Scene2DPainter;

class PrivacyFilter : public FrameListener, public FrameGenerator
{
    shared_ptr<QHashDataFrames> m_frames;
    QHashDataFrames m_framesCopy;
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
    int m_width = 640;
    int m_height = 480;
    bool m_paused = false;

public:
    static void convertQImage2ColorFrame(const QImage &input_img, ColorFramePtr output_img);

    PrivacyFilter();
    ~PrivacyFilter();
    void newFrames(const QHashDataFrames dataFrames) override;
    void singleFrame(const QHashDataFrames dataFrames, int width, int height);
    void enableFilter(ColorFilter filterType);
    void captureImage();
    void resize(int width, int height);
    void pause();

protected:
    void initialise(int width = 640, int height = 480);
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
