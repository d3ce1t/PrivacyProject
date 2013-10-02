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
    TutorialApplication.cpp \
    BaseApplication.cpp

HEADERS += \
    TutorialApplication.h \
    BaseApplication.h \
    common/SdkTrays.h \
    common/SdkSample.h \
    common/SdkCameraMan.h \
    common/SamplePlugin.h \
    common/SampleContext.h \
    common/Sample.h \
    common/FileSystemLayer.h

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
