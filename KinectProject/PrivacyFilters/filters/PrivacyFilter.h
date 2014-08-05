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
#include "types/Histogram2D.h"

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
    void approach1();
    void approach2();
    void approach3();
    void approach4();
    void dilateUserMask(uint8_t *labels);

    template <class T>
    shared_ptr<Histogram2D> computeHistogram(cv::Mat inputImg, cv::Mat mask);

    template <class T>
    cv::Mat randomSampling(cv::Mat inputImg, int n, cv::Mat mask = cv::Mat());

    cv::Mat convertRGB2Log2D(cv::Mat inputImg);
    cv::Mat calcHistogram(shared_ptr<ColorFrame> colorFrame, shared_ptr<MaskFrame> mask);
    std::vector<cv::Rect> faceDetection(cv::Mat frameGray, bool equalised = false);
    cv::Mat computeIntegralImage(cv::Mat image);

    template <class T>
    cv::Mat interleaveMatChannels(cv::Mat inputMat, cv::Mat mask = cv::Mat(), int type = CV_32SC1);

    double computeOccupancy(shared_ptr<MaskFrame> mask, int *outNumPixels = nullptr);
    void computeUpperAndLowerMasks(const cv::Mat input_img, cv::Mat &upper_mask, cv::Mat &lower_mask, const cv::Mat mask = cv::Mat()) const;

    template <class T>
    void create2DCoordImage(cv::Mat input_img, cv::Mat &output_img, int size[], float input_range[],
                            bool init_output = false, cv::Vec3b color = cv::Vec3b(255, 255, 255)) const;

    void create2DCoordImage(const Histogram2D& histogram, cv::Mat& output_img, int size[], float input_range[],
                            bool init_output = false, cv::Vec3b color = cv::Vec3b(255, 255, 255)) const;

    cv::Mat createMask(cv::Mat input_img, int min_value, int *nonzero_counter = nullptr, bool filter = false) const;
    void denoiseImage(cv::Mat input_img, cv::Mat output_img) const;

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
