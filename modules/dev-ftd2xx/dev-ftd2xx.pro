TARGET = devlib-ftd2xx
TEMPLATE = lib
CONFIG -= qt

DEFINES += DEVICE_MODULE
SOURCES = dev.c
INCLUDEPATH = ../../devlib extlib

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
    win32:LIBS += -L../../devlib/debug -ldevlib
    unix:LIBS += -L../../devlib -L../../devlib/debug -ldevlib -lftd2xx
} else {
    win32:LIBS += -L../../devlib/release -ldevlib
    unix:LIBS += -L../../devlib -L../../devlib/release -ldevlib -lftd2xx
}

win32:LIBS += $$PWD/extlib/i386/ftd2xx.lib

# default rules for deployment.
unix:!android: target.path = /usr/local/lib
!isEmpty(target.path): INSTALLS += target
