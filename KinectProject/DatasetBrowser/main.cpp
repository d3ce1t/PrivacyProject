#include <QApplication>
#include "DatasetBrowser.h"
#include "viewer/InstanceViewer.h"
#include "viewer/QMLEnumsWrapper.h"
#include "Config.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // For QSettings
    QCoreApplication::setOrganizationName("DAI");
    QCoreApplication::setOrganizationDomain("web.ua.es/dai");
    QCoreApplication::setApplicationName("Dataset Browser");

    Config::getInstance()->disableFilters();

    DatasetBrowser w;
    w.show();
    return a.exec();
}
