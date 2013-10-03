#-------------------------------------------------
#
# Project created by QtCreator 2013-09-30T16:51:27
#
#-------------------------------------------------

QT       += core gui

TARGET = OgreTest

TEMPLATE = app

# Use C++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS = -std=c++11

SOURCES += \
    BaseApplication.cpp \
    openni/OpenNIRuntime.cpp \
    character/CharacterSample.cpp \
    main.cpp \
    character/SinbadCharacterController.cpp

HEADERS += \
    BaseApplication.h \
    character/SinbadCharacterController.h \
    character/CharacterSample.h \
    openni/OpenNIRuntime.h \
    common/SdkTrays.h \
    common/SdkCameraMan.h

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
PKGCONFIG += \
    OGRE \
    OGRE-RTShaderSystem \
    OGRE-Property \
    OGRE-Terrain \
    OGRE-Paging

# OIS
PKGCONFIG += OIS

# CoreLib
unix:!macx: LIBS += -L$$OUT_PWD/../CoreLib/ -lCoreLib
INCLUDEPATH += $$PWD/../CoreLib
DEPENDPATH += $$PWD/../CoreLib
unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/libCoreLib.a

# Copy all resources to build folder
Resources.path = $$OUT_PWD/resources
Resources.files = resources/*

# Copy all config files to build folder
Config.path = $$OUT_PWD
Config.files = config/*

NiTE.path = $$OUT_PWD/../
NiTE.files = config/NiTE.ini

# make install
INSTALLS += Resources Config
