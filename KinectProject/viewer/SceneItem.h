#ifndef SCENEITEM_H
#define SCENEITEM_H

#include <QOpenGLFunctions>

namespace dai {

class ScenePainter;

class SceneItem : public QOpenGLFunctions
{
    friend class ScenePainter;

public:
    SceneItem();
    int getZOrder() const;
    void renderItem(int pass = 1);
    ScenePainter *scene() const;
    void setBackgroundTex(GLuint id);
    int neededPasses() const;

protected:
    virtual void initialise() = 0;
    virtual void render(int pass) = 0;

    int m_neededPasses;

    // OpenGL identifiers
    GLuint                   m_fgTextureId;


private:
    int m_z_order; // 0 = first item to be drawn
    bool m_initialised;
    ScenePainter* m_scene;
};

} // End Namespace

#endif // SCENEITEM_H
