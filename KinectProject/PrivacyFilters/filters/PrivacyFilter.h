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
#include "ControlWindow.h"

namespace dai {

class PrivacyFilter : public FrameListener, public FrameGenerator
{
public:
    PrivacyFilter();
    ~PrivacyFilter();
    void initialise();
    void newFrames(const QHashDataFrames dataFrames) override;
    void enableFilter(ColorFilter filterType);
    ControlWindow m_control;

protected:
    void afterStop() override;
    shared_ptr<QHashDataFrames> allocateMemory() override;
    void produceFrames(QHashDataFrames& output) override;
    void freeResources();

private:
    void approach1(QHashDataFrames& frames);
    void approach2(QHashDataFrames& frames);
    void approach3(QHashDataFrames& frames);
    void approach4(QHashDataFrames& frames);
    void approach5(QHashDataFrames& frames);
    void approach6(QHashDataFrames& frames);
    void dilateUserMask(uint8_t *labels);


    template <typename T, int N>
    static bool compare(const cv::Mat& inputImg, const QList<Point<T,N>>& point_list, const cv::Mat& mask = cv::Mat());

    std::vector<cv::Rect> faceDetection(cv::Mat frameGray, bool equalised = false);

    template <class T, int N>
    void printHistogram(const Histogram<T, N>& hist, int n_elems = 0) const;

    template <class T>
    void create2DCoordImage(cv::Mat input_img, cv::Mat& output_img, int size[], float input_range[],
                            bool init_output = false, cv::Vec3b color = cv::Vec3b(255, 255, 255)) const;

    template <class T>
    void create2DCoordImage(const QList<Histogram2D<T>*>& hist_list, int n_items, const QList<cv::Vec3b>& color_list,
                            cv::Mat& output_img, float input_range[]) const;

    void create2DCoordImage(const QList< QList<Point2f>* >& input_list, const QList<cv::Vec3b>& color_list,
                            cv::Mat& output_img, float input_range[]) const;

    template <class T>
    void create2DColorPalette(const QList<const HistBin3D<T>*>& upper_hist, const QList<const HistBin3D<T>*>& lower_hist, cv::Mat& output_img) const;

    cv::Mat createMask(cv::Mat input_img, int min_value, int *nonzero_counter = nullptr, bool filter = false) const;

    shared_ptr<QHashDataFrames> m_frames;
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
