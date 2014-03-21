#-------------------------------------------------
#
# Project created by QtCreator 2014-03-13T07:38:15
#
#-------------------------------------------------

QT       += core network

QT       -= gui

CONFIG += c++11

TARGET = Test-Web-App
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    WebAppServer.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../dist/release/ -lQt-Web-Server
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../dist/debug/ -lQt-Web-Server
else:unix: LIBS += -L$$PWD/../../dist/debug/ -lQt-Web-Server

INCLUDEPATH += $$PWD/../../src/
DEPENDPATH += $$PWD/../../src/

HEADERS += \
    WebAppServer.h
