TEMPLATE = subdirs

SUBDIRS += \
    devlib \
    modules \
    tests

# check that Qt version is atleast 5.7 (includes Qt Quick Controls 2)
equals(QT_MAJOR_VERSION, 5) {
    greaterThan(QT_MINOR_VERSION, 6) {
        SUBDIRS += gui multimeter
    } else {
        warning("Will NOT compile graphical applications, at least Qt 5.7 required")
    }
} else {
    warning("Will NOT compile graphical applications, at least Qt 5.7 required")
}
