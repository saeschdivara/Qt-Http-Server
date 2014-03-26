#-------------------------------------------------
#
# Project created by QtCreator 2014-03-13T07:33:38
#
#-------------------------------------------------

QT       += network

QT       -= gui

CONFIG += c++11

TARGET = Qt-Web-Server
TEMPLATE = lib

DEFINES += QTWEBSERVER_LIBRARY

SOURCES += QtWebServer.cpp \
    QtWebThread.cpp \
    QtWebRequest.cpp \
    QtWebResponse.cpp \
    QtWebThreadController.cpp \
    private/Rfc1036.cpp \
    private/Asctime.cpp \
    private/Rfc1123.cpp

HEADERS += QtWebServer.h\
        qt-web-server_global.h \
    QtWebThread.h \
    QtWebRequest.h \
    QtWebResponse.h \
    QtWebThreadController.h \
    private/Rfc1123.h \
    private/Rfc1036.h \
    private/Asctime.h
