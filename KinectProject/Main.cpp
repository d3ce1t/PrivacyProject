#include <QApplication>
#include "MainWindow.h"
#include <iostream>
#include "types/Quaternion.h"
#include <QList>
#include "types/DataFrame.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow* window = new MainWindow();
    qRegisterMetaType<QList<dai::DataFrame*> >("QList<dai::DataFrame*>");
    window->show();
    return app.exec();
    //dai::Quaternion::test();
}
