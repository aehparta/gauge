TARGET = ScopeGui
TEMPLATE = app
QT += qml quick widgets quickcontrols2 opengl
QT -= network
RESOURCES += gui.qrc

SOURCES += main.cpp \
    Device.cpp \
    Channel.cpp \
    Devices.cpp \
    SignalCanvas.cpp

HEADERS += \
    Device.h \
    Channel.h \
    Devices.h \
    SignalCanvas.h

DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH = ../devlib

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
    LIBS += -L../devlib -L../devlib/debug -lscope
    # tell qt creator to find dynamic libs from these paths for development purposes
    LIBS += -L../modules/dev-test/debug -L../modules/dev-test
    LIBS += -L../modules/dev-ftd2xx/debug -L../modules/dev-ftd2xx
} else {
    LIBS += -L../devlib/release -lscope
}

# default rules for deployment.
unix:!android: target.path = /usr/local/bin
!isEmpty(target.path): INSTALLS += target

# additional import path used to resolve QML modules in Qt Creator's code model
#QML_IMPORT_PATH =
#QML_DESIGNER_IMPORT_PATH =

DISTFILES +=

