#include <QApplication>
#include "DatasetBrowser.h"
#include "Config.h"
#include "viewer/InstanceViewerWindow.h"

int main(int argc, char *argv[])
{
    InstanceViewerWindow::initResources();
    QApplication a(argc, argv);

    // For QSettings
    QCoreApplication::setOrganizationName("DAI");
    QCoreApplication::setOrganizationDomain("web.ua.es/dai");
    QCoreApplication::setApplicationName("Dataset Browser");

    dai::Config::getInstance()->disableFilters();

    DatasetBrowser w;
    w.show();
    return a.exec();
}
