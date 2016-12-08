#-------------------------------------------------
#
# Project created by QtCreator 2016-11-23T17:33:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = usb_viewer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    usb_service.cpp \
    usb_thread.cpp

HEADERS  += mainwindow.h \
    version.h \
    app_var.h \
    usb_service.h \
    usb_thread.h \
    firmware.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -LC:/Lib/libusb-1.0.20/MinGW32/dll/ -LC:/Lib/share/lib/ -llibusb-1.0.dll -llibnanomsg.dll
else:win32:CONFIG(debug, debug|release): LIBS += -LC:/Lib/libusb-1.0.20/MinGW32/dll/ -LC:/Lib/share/lib/ -llibusb-1.0.dll -llibnanomsg.dll

INCLUDEPATH += C:/Lib/libusb-1.0.20/include/libusb-1.0 C:/Lib/share/include
DEPENDPATH += C:/Lib/libusb-1.0.20/include/libusb-1.0 C:/Lib/share/include
