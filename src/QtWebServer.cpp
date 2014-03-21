#include "QtWebServer.h"

#include "QtWebThread.h"

#include <QtCore/QDebug>
#include <QtNetwork/QSslSocket>

class QtWebServerPrivate
{
    public:
        bool isUsingSecureConnections = false;
        int threadCounter = 0;
};

QtWebServer::QtWebServer() :
    QTcpServer(0),
    d_ptr(new QtWebServerPrivate)
{
}

QtWebServer::~QtWebServer()
{
    delete d_ptr;
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

    qDebug() << QThread::currentThread() << "incomingConnection";

    QtWebThread * thread = new QtWebThread(this);
    thread->setSecureSocket(d->isUsingSecureConnections);
    thread->setSocketHandle(handle);

    d->threadCounter++;

    thread->setObjectName(QString("Qt Web Worker %1#").arg(d->threadCounter));

    QObject::connect( thread, &QtWebThread::clientConnectionReady,
                      this, &QtWebServer::clientConnectionReady,
                      Qt::DirectConnection
                      );

    QObject::connect( thread, &QtWebThread::finishedThisRequest,
                      thread, &QtWebThread::terminate
                      );

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
