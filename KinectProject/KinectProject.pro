QT += qml quick gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KinectProject

# Use C++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS = -std=c++11

HEADERS += \
    MainWindow.h \
    openni/OpenNIDepthInstance.h \
    KMeans.h \
    DepthSeg.h \
    openni/OpenNIColorInstance.h \
    openni/OpenNIRuntime.h \
    openni/OpenNIUserInstance.h \
    openni/OpenNISkeletonInstance.h \
    openni/OpenNIBaseInstance.h

SOURCES += \
    Main.cpp \
    MainWindow.cpp \
    openni/OpenNIDepthInstance.cpp \
    KMeans.cpp \
    DepthSeg.cpp \
    openni/OpenNIColorInstance.cpp \
    openni/OpenNIRuntime.cpp \
    openni/OpenNIUserInstance.cpp \
    openni/OpenNISkeletonInstance.cpp \
    openni/OpenNIBaseInstance.cpp

OTHER_FILES += \
    glsl/textureVertex.vsh \
    glsl/textureFragment.fsh \
    glsl/simpleVertex.vsh \
    glsl/simpleFragment.fsh \
    glsl/userFragment.fsh \
    glsl/userVertex.vsh \
    glsl/dummyVertex.vsh \
    glsl/dummyFragment.fsh \
    glsl/depthVertex.vsh \
    glsl/depthFragment.fsh \
    qml/main.qml

RESOURCES += openglunderqml.qrc

FORMS += \
    mainwindow.ui

unix:!macx {
    # CoreLib
    PRE_TARGETDEPS += /home/jose/qt-workspace/build-CoreLib-Desktop_Qt_5_1_1_GCC_64bit-Release/libCoreLib.a
    LIBS += -L/home/jose/qt-workspace/build-CoreLib-Desktop_Qt_5_1_1_GCC_64bit-Release/ -lCoreLib
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

    # OpenCV2
    LIBS += -lopencv_core -lopencv_imgproc
    INCLUDEPATH += /usr/include/opencv/
    DEPENDPATH += /usr/include/opencv/

    # VTK
    # INCLUDEPATH += /usr/include/vtk-5.8
    # DEPENDPATH += /usr/include/vtk-5.8

    # Eigen
    # INCLUDEPATH += /usr/include/eigen3/
    # DEPENDPATH += /usr/include/eigen3/

    # PCL
    # LIBS += -lpcl_common -lpcl_visualization -lboost_system -lpcl_io -lvtkCommon -lvtkFiltering
    # INCLUDEPATH += /usr/local/include/pcl-1.7/
    # DEPENDPATH += /usr/local/include/pcl-1.7/
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

    # Boost
    LIBS += -L"C:/boost_1_54_0/stage/lib"
    INCLUDEPATH += "C:/boost_1_54_0"
    DEPENDPATH += "C:/boost_1_54_0"

    # OpenNI2
    LIBS += -L"C:/Program Files/OpenNI2/Lib" -lOpenNI2
    INCLUDEPATH += "C:/Program Files/OpenNI2/Include"
    DEPENDPATH += "C:/Program Files/OpenNI2/Include"

    # NiTE2
    LIBS += -L"C:/Program Files/PrimeSense/NiTE2/Lib" -lNiTE2
    INCLUDEPATH += "C:/Program Files/PrimeSense/NiTE2/Include"
    DEPENDPATH += "C:/Program Files/PrimeSense/NiTE2/Include"

    # OpenCV2
    LIBS += -L"C:/opencv/build/x64/vc11/lib" -lopencv_core246 -lopencv_imgproc246
    #INCLUDEPATH += "C:/opencv/build/include"
    #DEPENDPATH += "C:/opencv/build/include"
}
