#include <QApplication>
#include "Config.h"
#include "MainWindow.h"
#include "filters/PrivacyFilter.h"

int main(int argc, char *argv[])
{
    CoreLib_InitResources();
    PrivacyLib_InitResources();
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
