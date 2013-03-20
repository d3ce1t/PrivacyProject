#ifndef DEPTHSTREAMSCENE_H
#define DEPTHSTREAMSCENE_H

#include "AbstractScene.h"
#include <OpenNI.h>
#include <NiTE.h>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

#define MAX_DEPTH 10000

class QOpenGLShaderProgram;

class DepthStreamScene : public AbstractScene
{
public:
    DepthStreamScene();
    virtual ~DepthStreamScene();
    void initialise();
    void update(float t);
    void render();
    void resize( float w, float h );
    void computeVideoTexture(nite::UserTrackerFrameRef& userTrackerFrame);
    void setNativeResolution(int width, int height);
    void setMatrix(QMatrix4x4& matrix);

private:
    openni::RGB888Pixel* prepareFrameTexture(openni::RGB888Pixel* texture, unsigned int width, unsigned int height, nite::UserTrackerFrameRef userTrackedFrame);
    void loadVideoTexture(openni::RGB888Pixel* texture, GLsizei width, GLsizei height, GLuint glTextureId);
    void prepareShaderProgram();
    void prepareVertexBuffers();


    const int               textureUnit;
    QVector3D               colors[4];
    const int               colorCount;

    QMatrix4x4              matrix;
    int                     m_nativeWidth;
    int                     m_nativeHeight;
    float                   m_width;
    float                   m_height;
    float                   m_pDepthHist[MAX_DEPTH];
    openni::RGB888Pixel*	m_pTexMap;
    unsigned int            m_nTexMapX;
    unsigned int            m_nTexMapY;

    QOpenGLShaderProgram*   m_shaderProgram;
    QOpenGLBuffer           m_vertexPositionBuffer;
    QOpenGLBuffer           m_vertexTexCoordsBuffer;

    // OpenGL identifiers
    GLuint                  m_frameTexture; // Texture Object
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_posAttr; // Pos attr in the shader
    GLuint                  m_texCoord; // Texture coord in the shader
    GLuint                  m_texSampler; // Texture Sampler in the shader
    GLuint                  m_sampler; // Sampler

    // Settings Flags
    bool g_drawBackground;

};

#endif // DEPTHSTREAMSCENE_H
