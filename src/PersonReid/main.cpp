#include <QApplication>
#include <QTimer>
#include "Config.h"
#include "PersonReid.h"

int main(int argc, char *argv[])
{
    CoreLib_InitResources();
    QApplication a(argc, argv);
    dai::PersonReid personReidApp;
    QTimer::singleShot(0, &personReidApp, SLOT(execute()));
    return a.exec();
}
