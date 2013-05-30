#include <QtGui/QGuiApplication>
#include "SampleViewer.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QSurfaceFormat format;
    format.setSamples(4);

    SampleViewer window;
    window.setFormat(format);
    window.resize(640, 480);
    window.show();
    window.setAnimating(true);
    return app.exec();
}
