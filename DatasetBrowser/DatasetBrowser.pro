#-------------------------------------------------
#
# Project created by QtCreator 2013-09-18T13:29:56
#
#-------------------------------------------------

QT       += core gui quick
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DatasetBrowser
TEMPLATE = app

# Use C++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS = -std=c++11

HEADERS  += \
    DatasetBrowser.h

SOURCES += main.cpp\
    DatasetBrowser.cpp

OTHER_FILES += \
    qml/main.qml \
    glsl/userVertex.vsh \
    glsl/userFragment.fsh \
    glsl/textureVertex.vsh \
    glsl/textureFragment.fsh \
    glsl/simpleVertex.vsh \
    glsl/simpleFragment.fsh \
    glsl/dummyVertex.vsh \
    glsl/dummyFragment.fsh \
    glsl/depthVertex.vsh \
    glsl/depthFragment.fsh

RESOURCES += \
    openglunderqml.qrc

FORMS    += \
    DatasetBrowser.ui

unix:!macx {
    # CoreLib
    PRE_TARGETDEPS += /home/jose/qt-workspace/build-CoreLib-Desktop_Qt_5_1_1_GCC_64bit-Release/libCoreLib.a
    LIBS += -L/home/jose/qt-workspace/build-CoreLib-Desktop_Qt_5_1_1_GCC_64bit-Release/ -lCoreLib
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib

    # OpenCV2
    LIBS += -lopencv_core -lopencv_imgproc
    INCLUDEPATH += /usr/include/opencv/
    DEPENDPATH += /usr/include/opencv/
}

# CoreLib Dynamic
win32:CONFIG(release, debug|release): LIBS += -L"C:/Users/Jose Padilla/Desktop/build-CoreLib-Desktop_Qt_5_1_1_MSVC2012_OpenGL_64bit-Release/release" -lCoreLib
else:win32:CONFIG(debug, debug|release): LIBS += -L"C:/Users/Jose Padilla/Desktop/build-CoreLib-Desktop_Qt_5_1_1_MSVC2012_OpenGL_64bit-Debug/debug" -lCoreLib

# CoreLib Static
win32:CONFIG(release, debug|release): PRE_TARGETDEPS += "C:/Users/Jose Padilla/Desktop/build-CoreLib-Desktop_Qt_5_1_1_MSVC2012_OpenGL_64bit-Release/release/CoreLib.lib"
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += "C:/Users/Jose Padilla/Desktop/build-CoreLib-Desktop_Qt_5_1_1_MSVC2012_OpenGL_64bit-Debug/debug/CoreLib.lib"

win32 {
    INCLUDEPATH += $$PWD

    # CoreLib
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib

    # OpenCV2
    #PRE_TARGETDEPS +=  "C:/opencv/build/x64/vc11/staticlib/opencv_core246.lib"
    #PRE_TARGETDEPS +=  "C:/opencv/build/x64/vc11/staticlib/opencv_imgproc246.lib"
    LIBS += -L"C:/opencv/build/x64/vc11/lib" -lopencv_core246 -lopencv_imgproc246
    #INCLUDEPATH += "C:/opencv/build/include"
    #DEPENDPATH += "C:/opencv/build/include"
}
