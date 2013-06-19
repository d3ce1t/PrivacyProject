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

class ViewerPainter
{
public:
    explicit ViewerPainter(StreamInstance *instance, InstanceViewer* parent = 0);
    virtual ~ViewerPainter();
    virtual bool prepareNext() = 0;
    void setMatrix(QMatrix4x4& m_matrix);
    void renderNow();
    StreamInstance& instance() const;
    virtual DataFrame& frame() = 0;

protected:
    virtual void initialise() = 0;
    virtual void render() = 0;

    QMatrix4x4              m_matrix;
    StreamInstance*         m_instance;
    InstanceViewer*         m_viewer;

private:
    bool                    m_initialised;
};

} // End Namespace

#endif // ABSTRACTSCENE_H
