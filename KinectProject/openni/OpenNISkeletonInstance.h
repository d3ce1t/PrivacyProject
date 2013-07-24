#ifndef OPENNISKELETONINSTANCE_H
#define OPENNISKELETONINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/Skeleton.h"
#include <QFile>

namespace dai {

class OpenNISkeletonInstance : public StreamInstance
{
public:
    OpenNISkeletonInstance();
    virtual ~OpenNISkeletonInstance();
    bool is_open() const override;
    void setOutputFile(QString file);

protected:
    void openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(DataFrame& frame) override;

private:
    OpenNIRuntime*            m_openni;
    shared_ptr<dai::Skeleton> m_frameBuffer[2];
    QFile                     m_of;
    QString                   m_outputFile;
};

} // End Namespace

#endif // OPENNISKELETONINSTANCE_H
