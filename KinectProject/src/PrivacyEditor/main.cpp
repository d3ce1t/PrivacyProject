#include "MainWindow.h"
#include <QApplication>
#include "Config.h"
#include "filters/PrivacyFilter.h"

int main(int argc, char *argv[])
{
    CoreLib_InitResources();
    PrivacyLib_InitResources();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
