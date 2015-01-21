#ifndef BACKGROUNDITEM_H
#define BACKGROUNDITEM_H

#include "viewer/SceneItem.h"
#include "types/ColorFrame.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

namespace dai {

class BackgroundItem : public SceneItem
{
public:
    BackgroundItem();
    virtual ~BackgroundItem();
    void setBackground(shared_ptr<ColorFrame> background);

protected:
    void initialise() override;
    void render(int pass) override;

private:
    void prepareShaderProgram();
    void prepareVertexBuffer();

    //shared_ptr<ColorFrame>   m_background;

    QOpenGLShaderProgram*    m_shaderProgram;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer            m_positionsBuffer;
    QOpenGLBuffer            m_texCoordBuffer;

    // OpenGL identifiers
    GLuint                   m_bgTextureId;
    GLuint                   m_perspectiveMatrixUniform;
    GLuint                   m_posAttr; // Pos attr in the shader
    GLuint                   m_texCoordAttr; // Texture coord in the shader
    GLuint                   m_texBGSampler;
};

} // End Namespace

#endif // BACKGROUNDITEM_H
