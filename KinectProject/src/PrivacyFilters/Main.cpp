#include <QApplication>
#include "Config.h"
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    CoreLib_InitResources();
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
