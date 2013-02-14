#-------------------------------------------------
#
# Project created by QtCreator 2013-01-25T17:26:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DataSetViewer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    depthviewer.cpp \    
    dataset/DataSetInfo.cpp \
    dataset/Sample.cpp \
    dataset/MSR3Action3D.cpp \
    dataset/MSRDailyAct3D.cpp

HEADERS  += mainwindow.h \
    depthviewer.h \
    dataset/AbstractDataSet.h \
    dataset/DataSetInfo.h \
    dataset/Sample.h \
    dataset/MSR3Action3D.h \
    dataset/MSRDailyAct3D.h

FORMS    += mainwindow.ui

