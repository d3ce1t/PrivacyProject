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
    window.show();
    return app.exec();
}
