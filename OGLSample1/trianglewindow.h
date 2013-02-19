#ifndef TRIANGLEWINDOW_H
#define TRIANGLEWINDOW_H

#include <QtGui/QOpenGLShaderProgram>
#include "openglwindow.h"
#include <QElapsedTimer>

class TriangleWindow : public OpenGLWindow
{
public:
    TriangleWindow();

    void initialize();
    void render();
    void ComputePositionOffsets(float &fXOffset, float &fYOffset);

private:
    GLuint loadShader(GLenum type, const char *source);

    QElapsedTimer timer;
    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_offsetLocation;
    QOpenGLShaderProgram *m_program;
};

#endif // TRIANGLEWINDOW_H
