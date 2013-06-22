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
    explicit Painter(StreamInstance* instance, InstanceViewer* parent = 0);
    virtual ~Painter();
    void setMatrix(QMatrix4x4& m_matrix);
    void renderNow();
    virtual DataFrame& frame() = 0;
    StreamInstance*   instance() const;

protected:
    virtual void initialise() = 0;
    virtual void render() = 0;

    QMatrix4x4              m_matrix;
    InstanceViewer*         m_viewer;
    StreamInstance*         m_instance;

private:
    bool                    m_initialised;
};

} // End Namespace

#endif // ABSTRACTSCENE_H
