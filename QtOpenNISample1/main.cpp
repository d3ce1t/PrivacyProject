#include <QtGui/QGuiApplication>
#include "trianglewindow.h"
#include <OpenNI.h>

int main(int argc, char **argv)
{
    openni::Device device;
    openni::VideoStream depth, color;
    const char* deviceURI = openni::ANY_DEVICE;
    QGuiApplication app(argc, argv);
    QSurfaceFormat format;
    format.setSamples(4);

    try {
        if (openni::OpenNI::initialize() != openni::STATUS_OK)
            throw 1;

        if (device.open(deviceURI) != openni::STATUS_OK)
            throw 2;

        if (depth.create(device, openni::SENSOR_DEPTH) != openni::STATUS_OK)
            throw 3;

        if (depth.start() != openni::STATUS_OK)
            throw 4;

        if (color.create(device, openni::SENSOR_COLOR) != openni::STATUS_OK)
            throw 5;

        if (color.start() != openni::STATUS_OK)
            throw 6;

        if (!depth.isValid() || !color.isValid())
            throw 7;


        TriangleWindow window("Simple Viewer", device, depth, color);
        window.setFormat(format);
        window.resize(1280, 1024);
        window.show();
        window.setAnimating(true);
        return app.exec();
    }
    catch (int ex) {

        switch (ex) {
        case 1:
            printf("After initialization:\n%s\n", openni::OpenNI::getExtendedError());
            break;
        case 2:
            printf("Device open failed:\n%s\n", openni::OpenNI::getExtendedError());
            break;
        case 3:
            printf("SimpleViewer: Couldn't find depth stream:\n%s\n", openni::OpenNI::getExtendedError());
            break;
        case 4:
            printf("SimpleViewer: Couldn't start depth stream:\n%s\n", openni::OpenNI::getExtendedError());
            depth.destroy();
            break;
        case 5:
            printf("SimpleViewer: Couldn't find color stream:\n%s\n", openni::OpenNI::getExtendedError());
            break;
        case 6:
            printf("SimpleViewer: Couldn't start color stream:\n%s\n", openni::OpenNI::getExtendedError());
            color.destroy();
            break;
        case 7:
            printf("SimpleViewer: No valid streams. Exiting\n");
            break;
        }

        openni::OpenNI::shutdown();
        return 1;
    }
}
