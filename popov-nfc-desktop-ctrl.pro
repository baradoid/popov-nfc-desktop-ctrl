#-------------------------------------------------
#
# Project created by QtCreator 2018-07-21T01:00:41
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = popov-nfc-desktop-ctrl
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


unix{
SOURCES += \
        main.cpp \
        dialog.cpp \
    nfcworkerthread.cpp\
     libnfcworkerthread.cpp

HEADERS += \
        dialog.h \
    nfcworkerthread.h \
    libnfcworkerthread.h
}
win32{
SOURCES += \
        main.cpp \
        dialog.cpp \
    nfcworkerthread.cpp

HEADERS += \
        dialog.h \
    nfcworkerthread.h
}


FORMS += \
        dialog.ui

win32:LIBS +=libwinscard

unix{
INCLUDEPATH += /usr/include/PCSC
CONFIG += c++11
LIBS += -lpcsclite -lnfc
}

#LIBS += "D:\Program Files\Microsoft SDKs\Windows\v7.0A\Lib\WinSCard.Lib"

