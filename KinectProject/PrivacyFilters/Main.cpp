#include <QApplication>
#include "MainWindow.h"
#include "types/Quaternion.h"
#include <QList>
#include "types/DataFrame.h"
#include "types/SkeletonFrame.h"
#include "viewer/InstanceViewer.h"
#include "viewer/QMLEnumsWrapper.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    qmlRegisterType<InstanceViewer>("OpenGLUnderQML", 1, 0, "InstanceViewer");
    qmlRegisterUncreatableType<QMLEnumsWrapper>("edu.dai.kinect", 1, 0, "ColorFilter", "This exports SomeState enums to QML");
    qRegisterMetaType<QHashDataFrames>("QHashDataFrames");
    qRegisterMetaType<QList<shared_ptr<BaseInstance>>>("QList<shared_ptr<BaseInstance>>");
    qRegisterMetaType<shared_ptr<SkeletonFrame>>("shared_ptr<SkeletonFrame>");
    window.show();
    return app.exec();
    //dai::Quaternion::test();
}
