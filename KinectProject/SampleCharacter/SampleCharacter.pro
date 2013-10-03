#-------------------------------------------------
#
# Project created by QtCreator 2013-09-26T12:29:12
#
#-------------------------------------------------

QT       += core gui

TARGET = SampleCharacter
TEMPLATE = lib

# Use C++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS = -std=c++11


SOURCES += \
    character/Sinbad.cpp \
    character/CharacterSample.cpp \
    openni/OpenNIRuntime.cpp

HEADERS += \
    character/SinbadCharacterController.h \
    character/CharacterSample.h \
    common/SdkTrays.h \
    common/SdkSample.h \
    common/SdkCameraMan.h \
    common/SamplePlugin.h \
    common/Sample.h \
    openni/OpenNIRuntime.h

OTHER_FILES +=

# OpenNI2
LIBS += -L/opt/OpenNI-Linux-x64-2.2/Tools/ -lOpenNI2
INCLUDEPATH += /opt/OpenNI-Linux-x64-2.2/Include
DEPENDPATH += /opt/OpenNI-Linux-x64-2.2/Include

# NiTE2
LIBS += -L/opt/NiTE-Linux-x64-2.2/Redist/ -lNiTE2
INCLUDEPATH += /opt/NiTE-Linux-x64-2.2/Include
DEPENDPATH += /opt/NiTE-Linux-x64-2.2/Include

# OGRE
CONFIG += link_pkgconfig
PKGCONFIG += OGRE

# OIS
PKGCONFIG += OIS

# CoreLib
unix:!macx: LIBS += -L$$OUT_PWD/../CoreLib/ -lCoreLib
INCLUDEPATH += $$PWD/../CoreLib
DEPENDPATH += $$PWD/../CoreLib
unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/libCoreLib.a
