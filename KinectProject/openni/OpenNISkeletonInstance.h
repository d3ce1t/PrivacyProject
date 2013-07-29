#ifndef OPENNISKELETONINSTANCE_H
#define OPENNISKELETONINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/SkeletonFrame.h"
#include <QFile>
#include <QTextStream>

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
    shared_ptr<SkeletonFrame> m_frameBuffer[2];
    QFile                     m_of;
    QTextStream               m_ts;
    QString                   m_outputFile;
};

} // End Namespace

#endif // OPENNISKELETONINSTANCE_H
