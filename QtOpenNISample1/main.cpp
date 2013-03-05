#include <QtGui/QGuiApplication>
#include "trianglewindow.h"
#include <OpenNI.h>
#include <iostream>

using namespace std;

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

        if (color.create(device, openni::SENSOR_COLOR) != openni::STATUS_OK)
            throw 5;

        if (device.setDepthColorSyncEnabled(true) == openni::STATUS_OK)
            cout << "Frame Sync enabled" << endl;

        if (depth.start() != openni::STATUS_OK)
            throw 4;

        if (color.start() != openni::STATUS_OK)
            throw 6;

        if (!depth.isValid() || !color.isValid())
            throw 7;




        if (device.isImageRegistrationModeSupported(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR)) {
            cout << "Image Registration is Supported" << endl;

            openni::ImageRegistrationMode mode = device.getImageRegistrationMode();
            cout << "Current Mode: " << mode << endl;

            //device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);
        }

        TriangleWindow window(device, depth, color);
        window.setFormat(format);
        window.resize(640, 480);
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
