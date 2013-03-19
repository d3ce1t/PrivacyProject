QT += qml quick
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtQuickOpenNI_NiTE_Sample1

HEADERS += squircle.h \
    OniSampleUtilities.h \
    simpleshaderprogram.h \
    depthstreamscene.h \
    AbstractScene.h

SOURCES += squircle.cpp main.cpp \
    simpleshaderprogram.cpp \
    depthstreamscene.cpp

RESOURCES += openglunderqml.qrc


# OpenNI2
LIBS += -L/opt/OpenNI-2.1.0-x64/Tools/ -lOpenNI2
INCLUDEPATH += /opt/OpenNI-2.1.0-x64/Include
DEPENDPATH += /opt/OpenNI-2.1.0-x64/Include

# NiTE2
LIBS += -L/opt/NiTE-2.0.0/Redist/ -lNiTE2
INCLUDEPATH += /opt/NiTE-2.0.0/Include
DEPENDPATH += /opt/NiTE-2.0.0/Include
