#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    InstanceViewerWindow::initResources();
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
