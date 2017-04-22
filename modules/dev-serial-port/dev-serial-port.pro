TARGET = devlib-serial-port
TEMPLATE = lib
CONFIG -= qt

DEFINES += DEVICE_MODULE
SOURCES = dev.c
INCLUDEPATH = ../../devlib

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
    win32:LIBS += -L../../devlib/debug -ldevlib
    LIBS += -L../../devlib -L../../devlib/debug -ldevlib
} else {
    win32:LIBS += -L../../devlib/release -ldevlib
    unix:LIBS += -L../../devlib -L../../devlib/release -ldevlib
}

# default rules for deployment.
unix:!android: target.path = /usr/local/lib
!isEmpty(target.path): INSTALLS += target
