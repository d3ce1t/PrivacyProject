!include(../common.pri) {
    error("Couldn't find the common.pri file!")
}

QT       += core gui quick multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DatasetBrowser
TEMPLATE = app
CONFIG += link_prl

HEADERS  += \
    DatasetBrowser.h \
    SettingsDialog.h \
    DatasetSelector.h

SOURCES += main.cpp\
    DatasetBrowser.cpp \
    SettingsDialog.cpp \
    DatasetSelector.cpp

OTHER_FILES += \
    glsl/scene3d.fsh \
    glsl/scene3d.vsh \
    glsl/scene2d.fsh \
    glsl/scene2d.vsh \
    glsl/silhouette.fsh \
    glsl/silhouette.vsh

RESOURCES +=

FORMS    += \
    DatasetBrowser.ui \
    SettingsDialog.ui \
    DatasetSelector.ui

# Linux y MacOSX
unix {
    # CoreLib
    LIBS += -L$$BIN_PATH/ -lCoreLib
    PRE_TARGETDEPS += $$BIN_PATH/libCoreLib.a
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib
}

win32 {
    #load(moc)
    INCLUDEPATH += $$PWD

    # CoreLib
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib

    # CoreLib Dynamic
    LIBS += -L$$BIN_PATH/ -lCoreLib

    # CoreLib Static
    PRE_TARGETDEPS += $$BIN_PATH/CoreLib.lib
}
