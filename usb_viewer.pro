#-------------------------------------------------
#
# Project created by QtCreator 2016-11-23T17:33:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = usb_viewer
TEMPLATE = app


SOURCES += src/main.cpp\
    src/mainwindow.cpp \
    src/usb_service.cpp \
    src/usb_thread.cpp \
    src/qcustomplot.cpp \
    src/graphwidget.cpp \
    src/firmware.cpp

HEADERS  += src/mainwindow.h \
    src/version.h \
    src/usb_service.h \
    src/usb_thread.h \
    src/firmware.h \
    src/qcustomplot.h \
    src/graphwidget.h

FORMS    += src/mainwindow.ui \
    src/graphwidget.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libs/libusb-1.0.21/MinGW32/static -llibusb-1.0
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libs/libusb-1.0.21/MinGW32/static -llibusb-1.0

INCLUDEPATH += src libs/libusb-1.0.21/include/libusb-1.0
DEPENDPATH += libs/libusb-1.0.21/include/libusb-1.0

RESOURCES += \
    src/main.qrc
