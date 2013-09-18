#include <QApplication>
#include "DatasetBrowser.h"
#include "viewer/InstanceViewer.h"
#include "viewer/QMLEnumsWrapper.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DatasetBrowser w;
    qmlRegisterType<InstanceViewer>("OpenGLUnderQML", 1, 0, "InstanceViewer");
    qmlRegisterUncreatableType<QMLEnumsWrapper>("edu.dai.kinect", 1, 0, "ColorFilter", "This exports SomeState enums to QML");
    qRegisterMetaType<QHashDataFrames>("QHashDataFrames");
    qRegisterMetaType<QList<shared_ptr<BaseInstance>>>("QList<shared_ptr<BaseInstance>>");
    qRegisterMetaType<shared_ptr<SkeletonFrame>>("shared_ptr<SkeletonFrame>");
    w.show();
    return a.exec();
}
