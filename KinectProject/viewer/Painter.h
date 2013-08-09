#ifndef PAINTER_H
#define PAINTER_H

#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include "types/DataFrame.h"

#ifndef M_PI
    #define M_PI 3.14159265359
#endif

class InstanceViewer;

namespace dai {

class UserFrame;

class Painter : protected QOpenGLFunctions
{
public:
    explicit Painter(InstanceViewer *parent);
    virtual ~Painter();
    InstanceViewer* parent() const;
    void setMatrix(QMatrix4x4& m_matrix);
    void renderNow();
    void setMask1(shared_ptr<UserFrame> mask);
    void setMask2(shared_ptr<UserFrame> mask);
    virtual void prepareData(shared_ptr<DataFrame> frame) = 0;

protected:
    virtual void prepareShaderProgram() = 0;
    virtual void initialise() = 0;
    virtual void render() = 0;

    QOpenGLShaderProgram*   m_shaderProgram;
    QMatrix4x4              m_matrix;
    InstanceViewer*         m_viewer;
    shared_ptr<UserFrame>   m_mask1;
    shared_ptr<UserFrame>   m_mask2;

private:
    bool                    m_initialised;
};

} // End Namespace

#endif // PAINTER_H
