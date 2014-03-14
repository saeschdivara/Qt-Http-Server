#include "QtWebServer.h"

#include "QtWebThread.h"

#include <QtCore/QDebug>
#include <QtNetwork/QSslSocket>

class QtWebServerPrivate
{
    public:
        bool isUsingSecureConnections = false;
};

QtWebServer::QtWebServer() :
    QTcpServer(0),
    d_ptr(new QtWebServerPrivate)
{
}

void QtWebServer::setSecure(bool isSecure)
{
    Q_D(QtWebServer);

    d->isUsingSecureConnections = isSecure;
}

bool QtWebServer::isSecure() const
{
    Q_D(const QtWebServer);

    return d->isUsingSecureConnections;
}

void QtWebServer::incomingConnection(qintptr handle)
{
    Q_D(QtWebServer);

    qDebug() << QThread::currentThread();

    QtWebThread * thread = new QtWebThread(this);
    thread->setSecureSocket(d->isUsingSecureConnections);
    thread->setSocketHandle(handle);

    thread->start();

//    if ( d->isUsingSecureConnections ) {
//        QSslSocket *serverSocket = new QSslSocket;
//        if (serverSocket->setSocketDescriptor(handle)) {
//            connect(serverSocket, &QSslSocket::encrypted,
//                    this, &QtWebServer::ready
//                    );
//            serverSocket->startServerEncryption();
//        } else {
//            delete serverSocket;
//        }
//    }
//    else {
//        connect(serverSocket, &QSslSocket::encrypted,
//                this, &QtWebServer::ready
//                );
//        QTcpServer::incomingConnection(handle);
//    }
}
