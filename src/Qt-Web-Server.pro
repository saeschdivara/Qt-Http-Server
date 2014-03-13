#-------------------------------------------------
#
# Project created by QtCreator 2014-03-13T07:33:38
#
#-------------------------------------------------

QT       += network

QT       -= gui

TARGET = Qt-Web-Server
TEMPLATE = lib

DEFINES += QTWEBSERVER_LIBRARY

SOURCES += QtWebServer.cpp

HEADERS += QtWebServer.h\
        qt-web-server_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
