#-------------------------------------------------
#
# Project created by QtCreator 2013-02-13T11:06:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtOpenNISample1
TEMPLATE = app

SOURCES += main.cpp \
    openglwindow.cpp \
    SampleViewer.cpp

HEADERS  += \
    openglwindow.h \
    OniSampleUtilities.h \
    SampleViewer.h

FORMS    +=

OTHER_FILES += \
    fragment.fsh \
    vertex.vsh

RESOURCES +=

LIBS += -L/opt/OpenNI-2.2.0-x64/Tools/ -lOpenNI2
INCLUDEPATH += /opt/OpenNI-2.2.0-x64/Include
DEPENDPATH += /opt/OpenNI-2.2.0-x64/Include
