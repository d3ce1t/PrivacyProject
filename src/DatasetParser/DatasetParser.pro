!include(../common.pri) {
    error("Couldn't find the common.pri file!")
}

QT       += core
#QT       -= gui

TARGET = DatasetParser
TEMPLATE  = app
CONFIG   += console link_prl
CONFIG   -= app_bundle

SOURCES += main.cpp

unix {
    # CoreLib
    LIBS += -L$$BIN_PATH -lCoreLib
    PRE_TARGETDEPS += $$BIN_PATH/libCoreLib.a
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib

    # OpenNI2
    LIBS += -L$$(OPENNI2_REDIST) -lOpenNI2
    INCLUDEPATH += $$(OPENNI2_INCLUDE)
    DEPENDPATH += $$(OPENNI2_INCLUDE)

    # NiTE2
    #LIBS += -L/opt/NiTE-Linux-x64-2.2/Redist/ -lNiTE2
    INCLUDEPATH += /opt/NiTE-Linux-x64-2.2/Include
    DEPENDPATH += /opt/NiTE-Linux-x64-2.2/Include
}

win32 {
    load(moc)
    INCLUDEPATH += $$PWD

    # CoreLib
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib

    # CoreLib Dynamic
    LIBS += -L$$BIN_PATH -lCoreLib

    # CoreLib Static
    PRE_TARGETDEPS += $$BIN_PATH/CoreLib.lib
}
