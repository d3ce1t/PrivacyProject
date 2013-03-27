QT += qml quick
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KinectProject

HEADERS += \
    depthstreamscene.h \
    AbstractScene.h \
    basicusagescene.h \
    skeleton.h \
    grill.h \
    histogramscene.h \
    mainwindow.h \
    viewer.h \
    dataset/AbstractDataSet.h \
    dataset/DataSetInfo.h \
    dataset/MSR3Action3D.h \
    dataset/MSRDailyAct3D.h \
    dataset/Sample.h \
    dataset/datasetmanager.h \
    dataset/DataSet.h

SOURCES += main.cpp \
    depthstreamscene.cpp \
    basicusagescene.cpp \
    skeleton.cpp \
    grill.cpp \
    histogramscene.cpp \
    mainwindow.cpp \
    viewer.cpp \
    dataset/DataSetInfo.cpp \
    dataset/MSR3Action3D.cpp \
    dataset/MSRDailyAct3D.cpp \
    dataset/Sample.cpp \
    dataset/datasetmanager.cpp \
    dataset/DataSet.cpp

RESOURCES += openglunderqml.qrc


# OpenNI2
LIBS += -L/opt/OpenNI-2.1.0-x64/Tools/ -lOpenNI2
INCLUDEPATH += /opt/OpenNI-2.1.0-x64/Include
DEPENDPATH += /opt/OpenNI-2.1.0-x64/Include

# NiTE2
LIBS += -L/opt/NiTE-2.0.0/Redist/ -lNiTE2
INCLUDEPATH += /opt/NiTE-2.0.0/Include
DEPENDPATH += /opt/NiTE-2.0.0/Include

FORMS += \
    mainwindow.ui
