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
    void renderItem();
    ScenePainter *scene() const;

protected:
    virtual void initialise() = 0;
    virtual void render() = 0;

private:
    int m_z_order; // 0 = first item to be drawn
    bool m_initialised;
    ScenePainter* m_scene;
};

} // End Namespace

#endif // SCENEITEM_H
