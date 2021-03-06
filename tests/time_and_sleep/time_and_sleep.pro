TARGET = test-time-and-sleep
TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES = time_and_sleep.c
INCLUDEPATH = ../../devlib

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
    win32:LIBS += -L../../devlib/debug -ldevlib -L../../modules/dev-test/debug
    unix:LIBS += -L../../devlib -ldevlib -L../../modules/dev-test
} else {
    win32:LIBS += -L../../devlib/release -ldevlib -L../../modules/dev-test/release
    unix:LIBS += -L../../devlib -ldevlib -L../../modules/dev-test
}
