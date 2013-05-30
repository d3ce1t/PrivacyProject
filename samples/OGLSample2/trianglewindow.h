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

private:
    void resizeEvent(QResizeEvent *event);
    GLuint loadShader(GLenum type, const char *source);

    QElapsedTimer timer;
    QMatrix4x4 matrix;
    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_offset;
    GLuint m_perspectiveMatrix;
    float fFrustumScale;
    float fzNear;
    float fzFar;
    QOpenGLShaderProgram *m_program;
};

#endif // TRIANGLEWINDOW_H
