#ifndef SCENE3DPAINTER_H
#define SCENE3DPAINTER_H

#include "viewer/ScenePainter.h"

namespace dai {

class Scene3DPainter : public ScenePainter
{
public:
    Scene3DPainter();

protected:
    void initialise() override;
    void render() override;
};

} // End Namespace

#endif // SCENE3DPAINTER_H
