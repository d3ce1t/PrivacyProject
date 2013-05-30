#include <QtGui/QGuiApplication>
#include "trianglewindow.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(4);

    TriangleWindow window;
    window.setFormat(format);
    window.resize(640, 640);
    window.show();

    window.setAnimating(true);

    return app.exec();
}
