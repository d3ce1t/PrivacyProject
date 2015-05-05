#ifndef SILHOUETTEITEM_H
#define SILHOUETTEITEM_H

#include "viewer/SceneItem.h"
#include "types/MaskFrame.h"
#include "types/ColorFrame.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

#define USER_COLORS 6

namespace dai {

class SilhouetteItem : public SceneItem
{
public:
    enum SilhouetteEffect {
        EFFECT_NORMAL = 0,
        EFFECT_BLUR,
        EFFECT_PIXELATION,
        EFFECT_EMBOSS
    };

    SilhouetteItem();
    void setUser(shared_ptr<MaskFrame> user);
    void setDrawingEffect(SilhouetteEffect effect);

protected:
    void initialise() override;
    void render(int pass) override;

private:
    void renderFirstPass();
    void renderSecondPass();
    void prepareShaderProgram();
    void prepareVertexBuffer();
    float gaussFunction(float x, float y, float sigma);
    float gaussFunction(float x, float sigma);
    void createKernel(float sigma);


    SilhouetteEffect         m_drawingEffect;

    QOpenGLShaderProgram*    m_shaderProgram;
    shared_ptr<MaskFrame>    m_user;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer            m_positionsBuffer;
    QOpenGLBuffer            m_texCoordBuffer;
    float                    m_kernel[15];

    // OpenGL identifiers
    GLuint                   m_posAttr; // Pos attr in the shader
    GLuint                   m_texCoord; // Texture coord in the shader
    GLuint                   m_kernelUniform;
    GLuint                   m_stageUniform;
    GLuint                   m_textureSizeUniform;
    GLuint                   m_silhouetteEffectUniform;
    GLuint                   m_maskTextureId;
    GLuint                   m_texFGSampler;
    GLuint                   m_texMaskSampler; // Texture Sampler in the shader
};

} // End Namespace

#endif // SILHOUETTEITEM_H
