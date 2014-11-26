#include "BackgroundItem.h"
#include "viewer/ScenePainter.h"

namespace dai {

BackgroundItem::BackgroundItem()
    : SceneItem(ITEM_BACKGROUND)
    , m_shaderProgram(nullptr)
{
}

BackgroundItem::~BackgroundItem()
{
    if (m_shaderProgram) {
        m_shaderProgram->removeAllShaders();
        delete m_shaderProgram;
        m_shaderProgram = nullptr;
    }
}

void BackgroundItem::setBackground(shared_ptr<ColorFrame> background)
{
    //m_background = background;
    // Load Background
    glBindTexture(GL_TEXTURE_2D, m_bgTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, background->width(), background->height(), 0, GL_RGB, GL_UNSIGNED_BYTE, background->getDataPtr());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void BackgroundItem::initialise()
{
    // Load, compile and link the shader program
    prepareShaderProgram();

    // Init Vertex Buffer
    prepareVertexBuffer();

    // Create texture
    glGenTextures(1, &m_bgTextureId);

    // Configure ViewPort and Clear Screen
    /*glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glViewport(0, 0, 640, 480);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/
}

void BackgroundItem::render(int pass)
{
    Q_UNUSED(pass)

    // Init Each Frame (because QtQuick could change it)
    //glDepthRange(0.0f, 1.0f);
    //glDepthMask(GL_TRUE);
    //glDepthFunc(GL_LEQUAL);
    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    //glEnable(GL_BLEND);
    //glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw Background
    m_shaderProgram->bind();
    //m_shaderProgram->setUniformValue(m_perspectiveMatrixUniform, m_matrix);

    m_vao.bind();

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_bgTextureId);
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_vao.release();
    m_shaderProgram->release();

    glDisable(GL_TEXTURE_2D);
}

void BackgroundItem::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/glsl/background.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/glsl/background.fsh");
    m_shaderProgram->bindAttributeLocation("posAttr", 0);
    m_shaderProgram->bindAttributeLocation("texCoord", 1);

    m_shaderProgram->link();

    m_posAttr = m_shaderProgram->attributeLocation("posAttr");
    m_texCoordAttr = m_shaderProgram->attributeLocation("texCoord");
    m_perspectiveMatrixUniform = m_shaderProgram->uniformLocation("perspectiveMatrix");
    m_texBGSampler = m_shaderProgram->uniformLocation("texBackground");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_texBGSampler, 0);
    m_shaderProgram->setUniformValue(m_perspectiveMatrixUniform, m_matrix);
    m_shaderProgram->release();
}

void BackgroundItem::prepareVertexBuffer()
{
    float vertexData[] = {
        0.0, (float) 480,
        (float) 640, (float) 480,
        (float) 640, 0,
        0.0, 0.0
    };

    float texCoordsData[] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };

    m_vao.create();
    m_vao.bind();

    m_positionsBuffer.create(); // Create a vertex buffer
    m_positionsBuffer.setUsagePattern(QOpenGLBuffer::StreamDraw);
    m_positionsBuffer.bind();
    m_positionsBuffer.allocate(vertexData, 4*2*sizeof(float));
    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->setAttributeBuffer(m_posAttr, GL_FLOAT, 0, 2 );
    m_positionsBuffer.release();

    m_texCoordBuffer.create();
    m_texCoordBuffer.setUsagePattern(QOpenGLBuffer::StreamDraw);
    m_texCoordBuffer.bind();
    m_texCoordBuffer.allocate(texCoordsData, 4*2*sizeof(float));
    m_shaderProgram->enableAttributeArray(m_texCoordAttr);
    m_shaderProgram->setAttributeBuffer(m_texCoordAttr, GL_FLOAT, 0, 2 );
    m_texCoordBuffer.release();

    m_vao.release();
}

} // End Namespace
