#include <QCoreApplication>
#include <QtWebServer.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QtWebServer server;

    server.listen(QHostAddress::Any, 3333);

    return a.exec();
}
