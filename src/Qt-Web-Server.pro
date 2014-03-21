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
    QtWebResponse.cpp

HEADERS += QtWebServer.h\
        qt-web-server_global.h \
    QtWebThread.h \
    QtWebRequest.h \
    QtWebResponse.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
