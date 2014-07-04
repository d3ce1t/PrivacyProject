QT += core qml quick gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PrivacyFilters
TEMPLATE = app
CONFIG += link_prl

# Use C++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS = -std=c++11

HEADERS += \
    MainWindow.h \
    openni/OpenNIDepthInstance.h \
    openni/OpenNIColorInstance.h \
    openni/OpenNIRuntime.h \
    ogre/SinbadCharacterController.h \
    ogre/cameranodeobject.h \
    ogre/OgreScene.h \
    openni/OpenNIUserTrackerInstance.h \
    ogre/OgrePointCloud.h \
    filters/PrivacyFilter.h

SOURCES += \
    Main.cpp \
    MainWindow.cpp \
    openni/OpenNIDepthInstance.cpp \
    openni/OpenNIColorInstance.cpp \
    openni/OpenNIRuntime.cpp \
    ogre/SinbadCharacterController.cpp \
    ogre/cameranodeobject.cpp \
    ogre/OgreScene.cpp \
    openni/OpenNIUserTrackerInstance.cpp \
    ogre/OgrePointCloud.cpp \
    filters/PrivacyFilter.cpp

OTHER_FILES += \
    qml/main.qml \
    glsl/scene3d.fsh \
    glsl/scene3d.vsh \
    glsl/silhouette.fsh \
    glsl/silhouette.vsh \
    glsl/scene2d.fsh \
    glsl/scene2d.vsh \
    glsl/skeleton.fsh \
    glsl/skeleton.vsh

RESOURCES += openglunderqml.qrc

FORMS += \
    mainwindow.ui

unix:!macx {
    # CoreLib
    LIBS += -L$$OUT_PWD/../CoreLib/ -lCoreLib
    PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/libCoreLib.a
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib

    # QmlOgreLib
    LIBS += -L$$OUT_PWD/../QmlOgreLib/ -lQmlOgre
    PRE_TARGETDEPS += $$OUT_PWD/../QmlOgreLib/libQmlOgre.a
    INCLUDEPATH += $$PWD/../QmlOgreLib
    DEPENDPATH += $$PWD/../QmlOgreLib

    # Ogre
    CONFIG += link_pkgconfig
    PKGCONFIG += OGRE

    # Boost
    LIBS += -lboost_system

    # OpenNI2
    LIBS += -L/opt/OpenNI-Linux-x64-2.2/Tools/ -lOpenNI2
    INCLUDEPATH += /opt/OpenNI-Linux-x64-2.2/Include
    DEPENDPATH += /opt/OpenNI-Linux-x64-2.2/Include

    # NiTE2
    LIBS += -L/opt/NiTE-Linux-x64-2.2/Redist/ -lNiTE2
    INCLUDEPATH += /opt/NiTE-Linux-x64-2.2/Include
    DEPENDPATH += /opt/NiTE-Linux-x64-2.2/Include

    # OpenCV2
    LIBS += -lopencv_core -lopencv_imgproc
    INCLUDEPATH += /usr/include/opencv/
    DEPENDPATH += /usr/include/opencv/
}

win32 {
    # ensure QMAKE_MOC contains the moc executable path
    load(moc)

    INCLUDEPATH += $$PWD

    # CoreLib
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib
    # Link Dynamic
    CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../CoreLib/release/ -lCoreLib
    else:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../CoreLib/debug/ -lCoreLib
    # Link Static
    !win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/release/CoreLib.lib
    else:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/debug/CoreLib.lib

    # QmlOgreLib
    INCLUDEPATH += $$PWD/../QmlOgreLib
    DEPENDPATH += $$PWD/../QmlOgreLib
    # Link Dynamic
    CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QmlOgreLib/release/ -lQmlOgre
    else:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QmlOgreLib/debug/ -lQmlOgre
    # Link Static
    !win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QmlOgreLib/release/QmlOgre.lib
    else:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QmlOgreLib/debug/QmlOgre.lib

    # Ogre
    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error(PrivacyFilters needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
    } else {
        message(Using Ogre libraries in $$OGREDIR)
        INCLUDEPATH += $$OGREDIR/include/OGRE
        INCLUDEPATH += $$OGREDIR/include/OGRE/RenderSystems/GL

        BOOSTDIR = $$OGREDIR/boost
        !isEmpty(BOOSTDIR) {
            INCLUDEPATH += $$BOOSTDIR
        }
    }

    # OpenNI2
    contains(QMAKE_TARGET.arch, x86_64) {
        LIBS += -L"C:/Program Files/OpenNI2/Lib" -lOpenNI2
        INCLUDEPATH += "C:/Program Files/OpenNI2/Include"
        DEPENDPATH += "C:/Program Files/OpenNI2/Include"
    }
    contains(QMAKE_TARGET.arch, x86) {
        LIBS += -L"C:/Program Files (x86)/OpenNI2/Lib" -lOpenNI2
        INCLUDEPATH += "C:/Program Files (x86)/OpenNI2/Include"
        DEPENDPATH += "C:/Program Files (x86)/OpenNI2/Include"
    }

    # NiTE2
    contains(QMAKE_TARGET.arch, x86_64) {
        LIBS += -L"C:/Program Files/PrimeSense/NiTE2/Lib" -lNiTE2
        INCLUDEPATH += "C:/Program Files/PrimeSense/NiTE2/Include"
        DEPENDPATH += "C:/Program Files/PrimeSense/NiTE2/Include"
    }
    contains(QMAKE_TARGET.arch, x86) {
        LIBS += -L"C:/Program Files (x86)/PrimeSense/NiTE2/Lib" -lNiTE2
        INCLUDEPATH += "C:/Program Files (x86)/PrimeSense/NiTE2/Include"
        DEPENDPATH += "C:/Program Files (x86)/PrimeSense/NiTE2/Include"
    }

    # OpenCV2
    INCLUDEPATH += "C:/opt/opencv-2.4.9/include"
    DEPENDPATH += "C:/opt/opencv-2.4.9/include"
    contains(QMAKE_TARGET.arch, x86_64):LIBS += -L"C:/opt/opencv-2.4.9/x64/vc11/lib" -lopencv_core249 -lopencv_imgproc249
    contains(QMAKE_TARGET.arch, x86):LIBS += -L"C:/opt/opencv-2.4.9/x86/vc11/lib" -lopencv_core249 -lopencv_imgproc249

    # OgreDLLs
    #CONFIG(release, debug|release): DESTDIR = $$OUT_PWD/release
    #CONFIG(debug, debug|release): DESTDIR = $$OUT_PWD/debug
    #OgreDLL.path = $$DESTDIR
    #OgreDLL.files += C:/opt/OgreSDK_vc11_v1-9-0/bin/debug/OgreMain_d.dll
    #OgreDLL.files += C:/opt/OgreSDK_vc11_v1-9-0/bin/debug//RenderSystem_GL_d.dll
    #INSTALLS += OgreDLL
}

CONFIG(release, debug|release): DESTDIR = $$OUT_PWD/release
CONFIG(debug, debug|release): DESTDIR = $$OUT_PWD/debug

# Copy all resources to build folder
unix!macosx:Resources.path = $$OUT_PWD/resources
win32:Resources.path = $$DESTDIR/resources
Resources.files = ../OgreData/*

# Copy all config files to build folder
unix!macosx:Config.path = $$OUT_PWD
unix!macosx:Config.files = config/linux/*
win32:Config.path = $$DESTDIR
win32:CONFIG(release, debug|release):Config.files = config/win/release/*
win32:CONFIG(debug, debug|release):Config.files = config/win/debug/*

# Copy OpenCV dll
win32:OPENCV_DIR = C:\opt\opencv-2.4.9\x86\vc11\bin
win32:OpenCV.path = $$DESTDIR
win32:OpenCV.files = $$OPENCV_DIR/opencv_core249.dll $$OPENCV_DIR/opencv_imgproc249.dll

# Copy Ogre dll
win32:CONFIG(release, debug|release): OGRE_DIR = C:\opt\OgreSDK_vc11_v1-9-0\bin\Release
win32:CONFIG(debug, debug|release): OGRE_DIR = C:\opt\OgreSDK_vc11_v1-9-0\bin\debug
win32:Ogre.path = $$DESTDIR
win32:Ogre.files = $$OGRE_DIR/OgreMain_d.dll $$OGRE_DIR/RenderSystem_GL_d.dll $$OGRE_DIR/OgreMain.dll $$OGRE_DIR/RenderSystem_GL.dll

# Copy OpenNI dll
win32:OPENNI_DIR = "C:\Program Files (x86)\OpenNI2\Redist"
win32:OpenNI.path = $$DESTDIR
win32:OpenNI.files = $$OPENNI_DIR/OpenNI2.dll $$OPENNI_DIR/OpenNI2

# Copy NiTE dll
win32:NITE_DIR = "C:\Program Files (x86)\PrimeSense\NiTE2\Redist"
win32:NiTE.path = $$DESTDIR
win32:NiTE.files = $$NITE_DIR/NiTE2.dll

# make install
INSTALLS += Resources Config
win32:INSTALLS += OpenCV Ogre OpenNI NiTE
