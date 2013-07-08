#include <QApplication>
#include "MainWindow.h"
#include <iostream>
#include "types/Quaternion.h"
#include <QList>
#include "types/DataFrame.h"
#include "viewer/InstanceViewer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    qmlRegisterType<InstanceViewer>("OpenGLUnderQML", 1, 0, "InstanceViewer");
    qRegisterMetaType<QList<dai::DataFrame*> >("QList<dai::DataFrame*>");
    window.show();
    return app.exec();
    //dai::Quaternion::test();
}
