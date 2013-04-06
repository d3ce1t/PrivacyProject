#ifndef DEPTHSTREAMSCENE_H
#define DEPTHSTREAMSCENE_H

#include "AbstractScene.h"
#include <OpenNI.h>
#include <NiTE.h>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include "DepthFrame.h"

#define MAX_DEPTH 10000

class QOpenGLShaderProgram;

class DepthStreamScene : public QObject, AbstractScene
{
    Q_OBJECT

    Q_PROPERTY(bool drawBackground  READ getDrawBackgroundFlag   WRITE setDrawBackgroundFlag  NOTIFY changeOfStatus)
    Q_PROPERTY(int overlayMode  READ getOverlayMode              WRITE setOverlayMode NOTIFY changeOfStatus)


public:
    DepthStreamScene();
    virtual ~DepthStreamScene();
    void initialise();
    void update(float t);
    void render();
    void resize( float w, float h );
    void computeVideoTexture(const dai::DepthFrame &frame);
    void setNativeResolution(int width, int height);
    void setMatrix(QMatrix4x4& m_matrix);
    bool getDrawBackgroundFlag() {return g_drawBackground;}
    int getOverlayMode() {return m_overlayMode;}

public slots:
    void setDrawBackgroundFlag(bool value);
    void setOverlayMode(int value);

signals:
    void changeOfStatus();

private:
    openni::RGB888Pixel* prepareFrameTexture(openni::RGB888Pixel* texture, const dai::DepthFrame &frame);
    void loadVideoTexture(openni::RGB888Pixel* texture, GLsizei width, GLsizei height, GLuint glTextureId);
    void prepareShaderProgram();
    //void prepareVertexBuffers();


    const int               textureUnit;
    QVector3D               colors[4];
    const int               colorCount;

    QMatrix4x4              m_matrix;
    int                     m_nativeWidth;
    int                     m_nativeHeight;
    float                   m_width;
    float                   m_height;
    float                   m_pDepthHist[MAX_DEPTH];
    openni::RGB888Pixel*	m_pTexMap;

    QOpenGLShaderProgram*   m_shaderProgram;

    // OpenGL identifiers
    GLuint                  m_frameTexture; // Texture Object
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_posAttr; // Pos attr in the shader
    GLuint                  m_texCoord; // Texture coord in the shader
    GLuint                  m_texSampler; // Texture Sampler in the shader
    GLuint                  m_sampler; // Sampler

    // Settings Flags
    bool                    g_drawBackground;
    int                     m_overlayMode; // 0 -> Depth, 1 -> Depth + RGB, 2 -> Only RGB

};

#endif // DEPTHSTREAMSCENE_H
