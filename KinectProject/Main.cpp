#include <QApplication>
#include "MainWindow.h"

#include <iostream>
#include "types/Quaternion.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow* window = new MainWindow();
    window->show();
    return app.exec();
    //dai::Quaternion::test();
}
