QT += qml quick
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtQuick_Sample1

HEADERS += \
    window.h \
    basicusagescene.h

SOURCES += main.cpp \
    window.cpp \
    basicusagescene.cpp

RESOURCES += openglunderqml.qrc

OTHER_FILES += \
    main.qml
