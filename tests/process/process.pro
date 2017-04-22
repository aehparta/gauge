TARGET = test-process
TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES = process.c
INCLUDEPATH = ../../devlib

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
    win32:LIBS += -L../../devlib/debug -ldevlib -L../../modules/dev-test/debug -L../../modules/dev-ftd2xx/debug -L../../modules/dev-serial-port/debug -L../../modules/trigger-edge/debug
    unix:LIBS += -L../../devlib -ldevlib -L../../modules/dev-test
} else {
    win32:LIBS += -L../../devlib/release -ldevlib -L../../modules/dev-test/release -L../../modules/dev-ftd2xx/release
    unix:LIBS += -L../../devlib -ldevlib -L../../modules/dev-test
}
