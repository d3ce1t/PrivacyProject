#ifndef ABSTRACTSCENE_H
#define ABSTRACTSCENE_H

#include <QMatrix4x4>
#include "../types/DataFrame.h"
#include "../types/StreamInstance.h"

#ifndef M_PI
    #define M_PI 3.14159265359
#endif

class InstanceViewer;

namespace dai {

class Painter
{
public:
    explicit Painter(InstanceViewer* parent);
    virtual ~Painter();
    void setMatrix(QMatrix4x4& m_matrix);
    void renderNow();
    virtual void prepareData(DataFrame* frame) = 0;
    virtual DataFrame& frame() = 0;

protected:
    virtual void initialise() = 0;
    virtual void render() = 0;

    QMatrix4x4              m_matrix;
    InstanceViewer*         m_viewer;

private:
    bool                    m_initialised;
};

} // End Namespace

#endif // ABSTRACTSCENE_H
