#include <QCoreApplication>
#include <QtWebServer.h>

#include "WebAppServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QtWebServer server;
    WebAppServer app;

    server.listen(QHostAddress::Any, 3333);

    QObject::connect( &server, &QtWebServer::clientConnectionReady,
                      &app, &WebAppServer::handleConnection,
                      Qt::DirectConnection
                      );

    return a.exec();
}
