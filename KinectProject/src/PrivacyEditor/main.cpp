#include "MainWindow.h"
#include <QApplication>
#include "Config.h"
#include <QDir>

int main(int argc, char *argv[])
{
    CoreLib_InitResources();
    QApplication a(argc, argv);
    MainWindow w;
    qDebug() << QDir::currentPath();
    w.show();
    return a.exec();
}
