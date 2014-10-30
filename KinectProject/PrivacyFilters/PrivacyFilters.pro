QT += core qml quick gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PrivacyFilters
TEMPLATE = app
CONFIG += link_prl
CONFIG += c++11

# Fix for Qt 5.3.1 on VS 2013 compiler, because Qt does not define a macro for this compiler
#win32 {
#    DEFINES += Q_COMPILER_INITIALIZER_LISTS
#}

HEADERS += \
    MainWindow.h \
    ogre/SinbadCharacterController.h \
    ogre/OgreScene.h \
    ogre/OgrePointCloud.h \
    filters/PrivacyFilter.h \
    ControlWindow.h

SOURCES += \
    Main.cpp \
    MainWindow.cpp \
    ogre/SinbadCharacterController.cpp \
    ogre/OgreScene.cpp \
    ogre/OgrePointCloud.cpp \
    filters/PrivacyFilter.cpp \
    ControlWindow.cpp

OTHER_FILES += \
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
    mainwindow.ui \
    ControlWindow.ui

unix:!macx {
    # CoreLib
    LIBS += -L$$OUT_PWD/../CoreLib/ -lCoreLib
    PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/libCoreLib.a
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib

    # OpenNI2
    LIBS += -L/opt/OpenNI-Linux-x64-2.2/Tools/ -lOpenNI2
    INCLUDEPATH += /opt/OpenNI-Linux-x64-2.2/Include
    DEPENDPATH += /opt/OpenNI-Linux-x64-2.2/Include

    # NiTE2
    LIBS += -L/opt/NiTE-Linux-x64-2.2/Redist/ -lNiTE2
    INCLUDEPATH += /opt/NiTE-Linux-x64-2.2/Include
    DEPENDPATH += /opt/NiTE-Linux-x64-2.2/Include

    # Ogre
    CONFIG += link_pkgconfig
    PKGCONFIG += OGRE

    # Boost
    LIBS += -lboost_system

    # OpenCV2
    #INCLUDEPATH += $$(OPENCV2_INCLUDE)
    #DEPENDPATH += $$(OPENCV2_INCLUDE)
    LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lopencv_photo -lopencv_features2d -lopencv_nonfree -lopencv_flann
}

win32 {
    # ensure QMAKE_MOC contains the moc executable path
    load(moc)

    INCLUDEPATH += $$PWD

    # CoreLib
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib

    # CoreLib Dynamic
    CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../CoreLib/release/ -lCoreLib
    else:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../CoreLib/debug/ -lCoreLib

    # CoreLib Static
    win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/release/libCoreLib.a
    else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/debug/libCoreLib.a
    else:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/release/CoreLib.lib
    else:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/debug/CoreLib.lib

    # Ogre
    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error(PrivacyFilters needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
    } else {
        INCLUDEPATH += $$OGREDIR/include/OGRE
        INCLUDEPATH += $$OGREDIR/include/OGRE/RenderSystems/GL
        CONFIG(release, debug|release):LIBS += -L$$OGREDIR/lib/release -L$$OGREDIR/lib/release/opt -lOgreMain -lRenderSystem_GL
        else:CONFIG(debug, debug|release):LIBS += -L$$OGREDIR/lib/debug -L$$OGREDIR/lib/debug/opt -lOgreMain_d -lRenderSystem_GL_d
    }

    # Boost
    BOOSTDIR = $$(BOOST_INCLUDEDIR)
    BOOSTLIB = $$(BOOST_LIBRARYDIR)
    !isEmpty(BOOSTDIR) {
        INCLUDEPATH += $$BOOSTDIR
        win32-g++:CONFIG(release, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-mgw48-mt-1_56 -lboost_thread-mgw48-mt-1_56
        else:win32-g++:CONFIG(debug, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-mgw48-mt-d-1_56 -lboost_thread-mgw48-mt-d-1_56
        else:CONFIG(release, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-vc120-mt-1_56 -lboost_thread-vc120-mt-1_56
        else:CONFIG(debug, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-vc120-mt-gd-1_56 -lboost_thread-vc120-mt-gd-1_56
    }

    # OpenNI2
    #LIBS += -L$$(OPENNI2_LIB) -lOpenNI2
    INCLUDEPATH += $$(OPENNI2_INCLUDE)
    DEPENDPATH += $$(OPENNI2_INCLUDE)

    # NiTE2
    #LIBS += -L$$(NITE2_LIB) -lNiTE2
    INCLUDEPATH += $$(NITE2_INCLUDE)
    DEPENDPATH += $$(NITE2_INCLUDE)

    # OpenCV2
    INCLUDEPATH += $$(OPENCV2_INCLUDE)
    DEPENDPATH += $$(OPENCV2_INCLUDE)
    CONFIG(release, debug|release):LIBS += -L$$(OPENCV2_LIB) -lopencv_core249 -lopencv_imgproc249 -lopencv_highgui249 -lopencv_objdetect249 -lopencv_photo249
    else:CONFIG(debug, debug|release):LIBS += -L$$(OPENCV2_LIB) -lopencv_core249d -lopencv_imgproc249d -lopencv_highgui249d -lopencv_objdetect249d -lopencv_photo249d
}

CONFIG(release, debug|release): DESTDIR = $$OUT_PWD
else:CONFIG(debug, debug|release): DESTDIR = $$OUT_PWD

# Install Linux Files
unix:!macx {
    # Ogre Resources
    Resources.path = $$OUT_PWD/resources
    Resources.files = ../OgreData/*

    # Config Files
    Config.path = $$OUT_PWD
    Config.files = ../config/linux/*
}

# Install Win Files
win32 {
    # Ogre Resources
    Resources.path = $$DESTDIR/resources
    Resources.files = ../OgreData/*

    # Config Files
    Config.path = $$DESTDIR
    CONFIG(release, debug|release):Config.files = ../config/win/release/*
    else:CONFIG(debug, debug|release):Config.files = ../config/win/debug/*

    # OpenCV dll
    OpenCV.path = $$DESTDIR
    win32-g++:CONFIG(release, debug|release):OpenCV.files = $$(OPENCV2_BIN)/libopencv_core249.dll $$(OPENCV2_BIN)/libopencv_imgproc249.dll $$(OPENCV2_BIN)/libopencv_highgui249.dll $$(OPENCV2_BIN)/libopencv_objdetect249.dll $$(OPENCV2_BIN)/libopencv_photo249.dll
    else:win32-g++:CONFIG(debug, debug|release):OpenCV.files = $$(OPENCV2_BIN)/libopencv_core249d.dll $$(OPENCV2_BIN)/libopencv_imgproc249d.dll $$(OPENCV2_BIN)/libopencv_highgui249d.dll $$(OPENCV2_BIN)/libopencv_objdetect249d.dll $$(OPENCV2_BIN)/libopencv_photo249d.dll
    else:CONFIG(release, debug|release):OpenCV.files = $$(OPENCV2_BIN)/opencv_core249.dll $$(OPENCV2_BIN)/opencv_imgproc249.dll $$(OPENCV2_BIN)/opencv_highgui249.dll $$(OPENCV2_BIN)/opencv_objdetect249.dll $$(OPENCV2_BIN)/opencv_photo249.dll
    else:CONFIG(debug, debug|release):OpenCV.files = $$(OPENCV2_BIN)/opencv_core249d.dll $$(OPENCV2_BIN)/opencv_imgproc249d.dll $$(OPENCV2_BIN)/opencv_highgui249d.dll $$(OPENCV2_BIN)/opencv_objdetect249d.dll $$(OPENCV2_BIN)/opencv_photo249d.dll

    # Ogre dll
    CONFIG(release, debug|release): OGRE_DIR = $$OGREDIR\bin\release
    else:CONFIG(debug, debug|release): OGRE_DIR = $$OGREDIR\bin\debug
    Ogre.path = $$DESTDIR
    Ogre.files = $$OGRE_DIR/OgreMain_d.dll $$OGRE_DIR/RenderSystem_GL_d.dll $$OGRE_DIR/OgreMain.dll $$OGRE_DIR/RenderSystem_GL.dll

    # OpenNI dll
    win32:OPENNI_DIR = $$(OPENNI2_REDIST)
    win32:OpenNI.path = $$DESTDIR
    win32:OpenNI.files = $$OPENNI_DIR/OpenNI2.dll $$OPENNI_DIR/OpenNI2

    # NiTE dll
    NITE_DIR = $$(NITE2_REDIST)
    NiTE.path = $$DESTDIR
    NiTE.files = $$NITE_DIR/NiTE2.dll

    INSTALLS += OpenCV Ogre OpenNI NiTE
}

# make install
INSTALLS += Resources Config
