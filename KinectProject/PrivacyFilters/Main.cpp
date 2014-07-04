#include <QApplication>
#include "MainWindow.h"
#include "ogreengine.h"
#include "Config.h"

int main(int argc, char *argv[])
{
    OgreEngine::initResources();
    QApplication app(argc, argv);
    dai::Config::getInstance();
    MainWindow window;
    window.show();
    return app.exec();
}
