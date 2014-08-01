#ifndef DEPTHFILTER_H
#define DEPTHFILTER_H

#include "playback/FrameListener.h"
#include "playback/FrameGenerator.h"
/*#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>
#include "viewer/Scene3DPainter.h"*/

namespace dai {

//
// Obtiene Depth y Skeleton y genera un frame de Color con ese contenido
//
class DepthFilter : public FrameListener, public FrameGenerator
{
public:
    //DepthFilter();
    ~DepthFilter();
    //void initialise();
    void newFrames(const QHashDataFrames dataFrames) override;

protected:
    //void afterStop() override;
    QHashDataFrames produceFrames() override;
    //void freeResources();

private:
    QHashDataFrames m_frames;
    //QOpenGLContext* m_glContext;
    //QOpenGLFunctions* m_gles;
    //QOffscreenSurface m_surface;
    //bool m_initialised;
    //Scene3DPainter* m_scene;
    //QOpenGLFramebufferObject* m_fboDisplay;
};

} // End Namespace

#endif // DEPTHFILTER_H
