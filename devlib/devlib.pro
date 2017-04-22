TARGET = devlib
TEMPLATE = lib
CONFIG -= qt

HEADERS = \
    debug.h \
    linkedlist.h \
    os.h \
    devlib.h \
    dev.h \
    devlib_process.h

SOURCES = \
    debug.c \
    os.c \
    devlib.c \
    devlib_process.c

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
} else {
    DEFINES +=
}

unix:LIBS += -ldl -lpthread

# default rules for deployment.
unix:!android: target.path = /usr/local/lib
!isEmpty(target.path): INSTALLS += target
