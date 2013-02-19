#-------------------------------------------------
#
# Project created by QtCreator 2013-02-13T11:06:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OGLSample2
TEMPLATE = app


SOURCES += main.cpp \
    openglwindow.cpp \
    trianglewindow.cpp

HEADERS  += \
    openglwindow.h \
    trianglewindow.h

FORMS    +=

OTHER_FILES += \
    fragment.fsh \
    vertex.vsh

RESOURCES += \
    shaders.qrc
