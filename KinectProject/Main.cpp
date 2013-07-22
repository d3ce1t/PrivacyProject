#include <QApplication>
#include "MainWindow.h"
#include "types/Quaternion.h"
#include <QList>
#include "types/DataFrame.h"
#include "viewer/InstanceViewer.h"
#include "viewer/QMLEnumsWrapper.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    qmlRegisterType<InstanceViewer>("OpenGLUnderQML", 1, 0, "InstanceViewer");
    qmlRegisterUncreatableType<QMLEnumsWrapper>("edu.dai.kinect", 1, 0, "ColorFilter", "This exports SomeState enums to QML");
    qRegisterMetaType<QList<shared_ptr<dai::DataFrame>> >("QList<shared_ptr<DataFrame>>");
    window.show();
    return app.exec();
    //dai::Quaternion::test();
}
