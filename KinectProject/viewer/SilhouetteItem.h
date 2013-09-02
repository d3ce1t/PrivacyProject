#ifndef SILHOUETTEITEM_H
#define SILHOUETTEITEM_H

#include "viewer/SceneItem.h"
#include "types/UserFrame.h"
#include "types/ColorFrame.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

#define USER_COLORS 6

namespace dai {

class SilhouetteItem : public SceneItem
{
public:
    SilhouetteItem();
    void setUser(shared_ptr<UserFrame> user);

protected:
    void initialise();
    void render();

private:
    static RGBColor staticUserColors[USER_COLORS];

    void prepareShaderProgram();
    void prepareVertexBuffer();
    void loadVideoTexture(void *texture, GLsizei width, GLsizei height, GLuint glTextureId);

    QOpenGLShaderProgram*    m_shaderProgram;
    shared_ptr<UserFrame>    m_user;
    RGBColor*                m_textureMask;
    const int                textureUnit;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer            m_positionsBuffer;
    QOpenGLBuffer            m_texCoordBuffer;

    // OpenGL identifiers
    GLuint                   m_frameTexture; // Texture Object
    GLuint                   m_perspectiveMatrix; // Matrix in the shader
    GLuint                   m_posAttr; // Pos attr in the shader
    GLuint                   m_texCoord; // Texture coord in the shader
    GLuint                   m_texSampler; // Texture Sampler in the shader
    GLuint                   m_sampler; // Sampler
};

} // End Namespace

#endif // SILHOUETTEITEM_H
