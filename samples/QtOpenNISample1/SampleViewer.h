#ifndef SAMPLEVIEWER_H
#define SAMPLEVIEWER_H

#include <QtGui/QOpenGLShaderProgram>
#include "openglwindow.h"
#include <QElapsedTimer>

#include <OpenNI.h>

#define MAX_DEPTH 10000

enum DisplayModes
{
    DISPLAY_MODE_OVERLAY,
    DISPLAY_MODE_DEPTH,
    DISPLAY_MODE_IMAGE
};

class SampleViewer : public OpenGLWindow
{
public:
    SampleViewer();
    virtual ~SampleViewer();
    void initialize();
    void render();

protected:
    virtual void keyPressEvent(QKeyEvent * ev);

    openni::VideoFrameRef	m_depthFrame;
    openni::VideoFrameRef	m_colorFrame;
    openni::Device			m_device;
    openni::VideoStream     m_depthStream;
    openni::VideoStream 	m_colorStream;
    openni::VideoStream**	m_streams;

private:
    void initOpenNI();

    float                   m_pDepthHist[MAX_DEPTH];
    unsigned int            m_nTexMapX;
    unsigned int            m_nTexMapY;
    DisplayModes            m_eViewState;
    openni::RGB888Pixel*	m_pTexMap;
    int                     m_width;
    int                     m_height;
};

#endif // SAMPLEVIEWER_H
