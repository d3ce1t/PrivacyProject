#include <RenderSystems/GL/OgreGLTexture.h>
#include <RenderSystems/GL/OgreGLFrameBufferObject.h>
#include <RenderSystems/GL/OgreGLFBORenderTexture.h>
#include "ogrerenderer.h"
#include <Ogre.h>
#include <QOpenGLFramebufferObject>

OgreRenderer::OgreRenderer(const OgreItem* viewer)
    : m_shaderProgram(nullptr)
    , m_engine(nullptr)
    , m_viewport(nullptr)
    , m_camera(nullptr)
    , m_renderTarget(nullptr)
    , m_texture(nullptr)
    , m_fbo(nullptr)
    , m_size(1, 1)
{
    m_viewer = const_cast<OgreItem*>(viewer);
    initializeOpenGLFunctions();
    prepareShaderProgram();
}

void OgreRenderer::setOgreEngine(OgreEngine* engine)
{
    m_engine = engine;
}

void OgreRenderer::setCamera(Ogre::Camera* camera)
{
    m_camera = camera;
}

void OgreRenderer::render()
{
    // Init Each Frame (because QtQuick could change it)
    glDepthRange(0.0f, 1.0f);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_TEXTURE_2D);

    // Configure ViewPort and Clear Screen
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shaderProgram->bind();

    float vertexData[] = {
        0.0, (float) m_size.height(),
        (float) m_size.width(), (float) m_size.height(),
        (float) m_size.width(), 0,
        0.0, 0.0
    };

    float texCoordsData[] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };

    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->enableAttributeArray(m_textCoordAttr);
    m_shaderProgram->setAttributeArray(m_posAttr, GL_FLOAT, vertexData, 2);
    m_shaderProgram->setAttributeArray(m_textCoordAttr, GL_FLOAT, texCoordsData, 2);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_texture->textureId());

    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_shaderProgram->release();

    // Update Ogre Scene
    if (m_renderTarget) {
        m_engine->activateOgreContext();
        m_renderTarget->update(true);
        m_engine->doneOgreContext();
    }

    // Restore
    glDisable(GL_TEXTURE_2D);
}

QOpenGLFramebufferObject* OgreRenderer::createFramebufferObject(const QSize &size)
{
    m_size = size;

    m_engine->activateOgreContext();

    if (m_renderTarget)
        Ogre::TextureManager::getSingleton().remove("RttTex");

    // First create Ogre RTT texture
    int samples = 4;
    m_rttTexture = Ogre::TextureManager::getSingleton().createManual("RttTex",
                                                                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                                    Ogre::TEX_TYPE_2D,
                                                                    m_size.width(),
                                                                    m_size.height(),
                                                                    0,
                                                                    Ogre::PF_R8G8B8A8,
                                                                    Ogre::TU_RENDERTARGET, 0, false,
                                                                    samples);

    m_renderTarget = m_rttTexture->getBuffer()->getRenderTarget();

    m_viewport = m_renderTarget->addViewport(m_camera);
    m_viewport->setDepthClear(1.0f);
    m_viewport->setBackgroundColour(Ogre::ColourValue(0.0f, 0.0f, 0.0f, 0.0f));
    m_viewport->setClearEveryFrame(true);
    m_viewport->setOverlaysEnabled(false);

    Ogre::Real aspectRatio = Ogre::Real(m_size.width()) / Ogre::Real(m_size.height());
    m_camera->setAspectRatio(aspectRatio);

    Ogre::GLTexture* nativeTexture = static_cast<Ogre::GLTexture *>(m_rttTexture.get());
    m_texture = m_engine->createTextureFromId(nativeTexture->getGLID(), size);

    m_engine->doneOgreContext();

    // Create my Qt Quick FBO
    QMatrix4x4 matrix;
    matrix.setToIdentity();
    matrix.ortho(0, m_size.width(), m_size.height(), 0, -1.0, 1.0);
    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_perspectiveMatrixUniform, matrix);
    m_shaderProgram->release();

    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    m_fbo = new QOpenGLFramebufferObject(m_size, format);
    return m_fbo;
}

void OgreRenderer::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/vertex.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/fragment.fsh");
    m_shaderProgram->bindAttributeLocation("posAttr", 0);
    m_shaderProgram->bindAttributeLocation("texCoord", 1);

    m_shaderProgram->link();

    m_posAttr = m_shaderProgram->attributeLocation("posAttr");
    m_textCoordAttr = m_shaderProgram->attributeLocation("texCoord");
    m_perspectiveMatrixUniform = m_shaderProgram->uniformLocation("perspectiveMatrix");
    m_texColorSampler = m_shaderProgram->uniformLocation("texForeground");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_texColorSampler, 0);
    m_shaderProgram->release();
}
