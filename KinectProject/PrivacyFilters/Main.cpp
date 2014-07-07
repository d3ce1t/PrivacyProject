#include <QApplication>
#include "MainWindow.h"
#include "ogreengine.h"

int main(int argc, char *argv[])
{
    InstanceViewerWindow::initResources();
    OgreEngine::initResources();
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
