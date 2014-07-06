#ifndef PRIVACYFILTER_H
#define PRIVACYFILTER_H

#include "playback/FrameListener.h"
#include "playback/FrameGenerator.h"
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>
#include "viewer/Scene2DPainter.h"

namespace dai {

class PrivacyFilter : public FrameListener, public FrameGenerator
{
public:
    PrivacyFilter();
    ~PrivacyFilter();
    void initialise();
    void newFrames(const QHashDataFrames dataFrames) override;

protected:
    QHashDataFrames produceFrames() override;

private:
    void dilateUserMask(uint8_t *labels);
    QHashDataFrames m_frames;
    QOpenGLContext* m_glContext;
    QOpenGLFunctions* m_gles;
    QOffscreenSurface m_surface;
    bool m_initialised;
    Scene2DPainter m_scene;
    QOpenGLFramebufferObject* m_fboDisplay;
};

} // End Namespace

#endif // PRIVACYFILTER_H
