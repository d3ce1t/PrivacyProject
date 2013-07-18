#ifndef PAINTER_H
#define PAINTER_H

#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include "../types/DataFrame.h"

#ifndef M_PI
    #define M_PI 3.14159265359
#endif

class InstanceViewer;

namespace dai {

class UserFrame;

class Painter : protected QOpenGLFunctions
{
public:
    explicit Painter(QOpenGLContext *context);
    virtual ~Painter();
    void setMatrix(QMatrix4x4& m_matrix);
    void renderNow();
    void setMask(shared_ptr<UserFrame> mask);
    virtual void prepareData(shared_ptr<DataFrame> frame) = 0;
    virtual DataFrame& frame() = 0;

protected:
    virtual void prepareShaderProgram() = 0;
    virtual void initialise() = 0;
    virtual void render() = 0;

    QOpenGLShaderProgram*   m_shaderProgram;
    QMatrix4x4              m_matrix;
    QOpenGLContext*         m_context;
    shared_ptr<UserFrame>   m_mask;

private:
    bool                    m_initialised;
};

} // End Namespace

#endif // PAINTER_H
