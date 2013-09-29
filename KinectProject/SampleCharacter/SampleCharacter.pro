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
LIBS += -lpthread -lOgreMain
INCLUDEPATH += /usr/include/OGRE
DEPENDPATH += /usr/include/OGRE

# OGRE Property
LIBS += -lOgreProperty
INCLUDEPATH += /usr/include/OGRE/Property
DEPENDPATH += /usr/include/OGRE/Property

# OGRE RTShaderSystem
LIBS += -lOgreRTShaderSystem
INCLUDEPATH += /usr/include/OGRE/RTShaderSystem
DEPENDPATH += /usr/include/OGRE/RTShaderSystem

# OGRE Paging
LIBS += -lOgrePaging
INCLUDEPATH += /usr/include/OGRE/Paging
DEPENDPATH += /usr/include/OGRE/Paging

# OGRE Terrain
LIBS += -lOgreTerrain
INCLUDEPATH += /usr/include/OGRE/Terrain
DEPENDPATH += /usr/include/OGRE/Terrain

# OGRE PCZ
LIBS += /usr/lib/x86_64-linux-gnu/OGRE-1.8.0/Plugin_PCZSceneManager.so
LIBS += /usr/lib/x86_64-linux-gnu/OGRE-1.8.0/Plugin_OctreeZone.so
INCLUDEPATH += /usr/include/OGRE/Plugins/PCZSceneManager
INCLUDEPATH += /usr/include/OGRE/Plugins/OctreeZone
DEPENDPATH += /usr/include/OGRE/Plugins/PCZSceneManager
DEPENDPATH += /usr/include/OGRE/Plugins/OctreeZone

# OIS
LIBS += -lOIS
INCLUDEPATH += /usr/include/ois
DEPENDPATH += /usr/include/ois

# CoreLib
unix:!macx: LIBS += -L$$OUT_PWD/../CoreLib/ -lCoreLib
INCLUDEPATH += $$PWD/../CoreLib
DEPENDPATH += $$PWD/../CoreLib
unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/libCoreLib.a
