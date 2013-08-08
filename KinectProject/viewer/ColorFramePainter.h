#ifndef COLOR_FRAME_PAINTER_H
#define COLOR_FRAME_PAINTER_H

#include "Painter.h"
#include <QOpenGLFramebufferObject>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include "types/ColorFrame.h"
#include <atomic>
#include "QMLEnumsWrapper.h"

class QOpenGLShaderProgram;

using namespace std;

namespace dai {

class ColorFramePainter : public Painter
{
public:
    ColorFramePainter(InstanceViewer* parent);
    virtual ~ColorFramePainter();
    void prepareData(shared_ptr<DataFrame> frame);
    void enableFilter(QMLEnumsWrapper::ColorFilter type);

protected:
    void initialise();
    void render();

private:
    void createFrameBuffer();
    void enableBGRendering();
    void renderBackground();
    void enableFilterRendering();
    void renderFilter();
    void displayRenderedTexture();
    void prepareShaderProgram();
    void prepareVertexBuffer();
    void loadVideoTexture(GLuint glTextureId, GLsizei width, GLsizei height, void *texture);
    void loadMaskTexture(GLuint glTextureId, GLsizei width, GLsizei height, void *texture);

    shared_ptr<ColorFrame>    m_frame;
    QMLEnumsWrapper::ColorFilter m_currentFilter = QMLEnumsWrapper::FILTER_DISABLED;
    atomic<int>               m_needLoading;

    // OpenGL Buffer
    shared_ptr<QOpenGLFramebufferObject>  m_fbo;
    shared_ptr<QOpenGLFramebufferObject>  m_fboFilter;
    QOpenGLVertexArrayObject              m_vao;
    QOpenGLBuffer                         m_positionsBuffer;
    QOpenGLBuffer                         m_texCoordBuffer;

    // OpenGL identifiers
    GLuint                   m_bgTextureId;
    GLuint                   m_fgTextureId;
    GLuint                   m_maskTextureId;

    // Shader identifiers
    GLuint                   m_perspectiveMatrixUniform;
    GLuint                   m_posAttr;
    GLuint                   m_textCoordAttr;
    GLuint                   m_stageUniform;
    GLuint                   m_currentFilterUniform;
    GLuint                   m_texColorSampler;
    GLuint                   m_texMaskSampler;
    GLuint                   m_texBackgroundSampler;
};

} // End Namespace

#endif // COLOR_FRAME_PAINTER_H
