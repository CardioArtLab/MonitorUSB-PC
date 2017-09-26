#-------------------------------------------------
#
# Project created by QtCreator 2016-11-23T17:33:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = usb_viewer
TEMPLATE = app


SOURCES += src/firmware.cpp \
    src/graphwidget.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/qcustomplot.cpp \
    src/usb_service.cpp \
    src/usb_thread.cpp \
    src/filter/pulseoximterfilter.cpp \
    src/filter/spo2filter.cpp \
    src/testwidget.cpp

HEADERS  += src/firmware.h \
    src/graphwidget.h \
    src/mainwindow.h \
    src/qcustomplot.h \
    src/usb_service.h \
    src/usb_thread.h \
    src/version.h \
    src/filter/basefilter.h \
    src/filter/pulseoximterfilter.h \
    src/filter/spo2filter.h \
    src/testwidget.h

FORMS    += src/mainwindow.ui \
    src/graphwidget.ui \
    src/testwidget.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libs/libusb-1.0.21/MinGW32/static -llibusb-1.0
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libs/libusb-1.0.21/MinGW32/static -llibusb-1.0

INCLUDEPATH += src libs/libusb-1.0.21/include/libusb-1.0
DEPENDPATH += libs/libusb-1.0.21/include/libusb-1.0

RESOURCES += \
    src/main.qrc
