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


SOURCES += main.cpp\
    DatasetBrowser.cpp

HEADERS  += \
    DatasetBrowser.h

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

win32 {
    INCLUDEPATH += $$PWD
}

# CoreLib Dynamic
win32:CONFIG(release, debug|release): LIBS += -L/home/jose/qt-workspace/build-CoreLib-Desktop_Qt_5_1_1_GCC_64bit-Release/ -lCoreLib
else:win32:CONFIG(debug, debug|release): LIBS += -L/home/jose/qt-workspace/build-CoreLib-Desktop_Qt_5_1_1_GCC_64bit-Debug/ -lCoreLib

# CoreLib Static
win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../../../home/jose/qt-workspace/build-CoreLib-Desktop_Qt_5_1_1_GCC_64bit-Release/release/CoreLib.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../../../home/jose/qt-workspace/build-CoreLib-Desktop_Qt_5_1_1_GCC_64bit-Release/debug/CoreLib.lib

win32 {
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib
}

win32 {
    # OpenCV2
    LIBS += -L"C:\opencv\build\x64\vc11\lib" -lopencv_core246 -lopencv_imgproc246
    INCLUDEPATH += "C:/opencv/build/include"
    DEPENDPATH += "C:/opencv/build/include"
}

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
