QT += core
QT += gui
QT += network
QT += qml
QT += quickwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

RC_ICONS = favicon.ico

CONFIG += c++11
CONFIG += static

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    node.cpp \
    utils.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    form.ui \
    mainwindow.ui \
    mainwindow_responsive.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RESOURCES += \
    resources.qrc
