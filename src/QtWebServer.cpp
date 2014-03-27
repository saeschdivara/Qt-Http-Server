#include "QtWebServer.h"

#include "QtWebThread.h"

#include <QtCore/QDebug>
#include <QtCore/QHash>
#include <QtCore/QQueue>
#include <QtNetwork/QSslSocket>

const int MAX_THREADS = QThread::idealThreadCount() * 2;

class QtWebServerPrivate
{
    public:
        bool isUsingSecureConnections = false;
        int threadCounter = 0;

        QHash<QtWebThread *, bool> threadList;
        QQueue<qintptr> connectionQueue;
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

void QtWebServer::threadAvailable()
{
    Q_D(QtWebServer);
    QtWebThread * thread = qobject_cast<QtWebThread *>(sender());

    if ( d->connectionQueue.size() > 0 ) {
        qintptr handle = d->connectionQueue.dequeue();
        thread->setSocketHandle(handle);

        thread->invokeNewPower();
    }
    else {
        d->threadList[thread] = true;
    }
}

void QtWebServer::incomingConnection(qintptr handle)
{
    Q_D(QtWebServer);

    bool hasFreeThread = false;

    for ( QtWebThread * thread : d->threadList.keys() ) {
        if ( d->threadList.value(thread) ) {
            d->threadList[thread] = false;
            hasFreeThread = true;

            thread->setSecureSocket(d->isUsingSecureConnections);
            thread->setSocketHandle(handle);

            thread->invokeNewPower();

            break;
        }
    }

    if ( !hasFreeThread ) {

        if ( d->threadCounter == MAX_THREADS ) {
            d->connectionQueue.enqueue(handle);
        }
        else {
            QtWebThread * thread = new QtWebThread(this);
            thread->setSecureSocket(d->isUsingSecureConnections);
            thread->setSocketHandle(handle);

            d->threadList.insert(thread, false);

            d->threadCounter++;

            thread->setObjectName(QString("Qt Web Worker %1#").arg(d->threadCounter));

            QObject::connect( thread, &QtWebThread::clientConnectionReady,
                              this, &QtWebServer::clientConnectionReady,
                              Qt::DirectConnection
                              );

            QObject::connect( thread, &QtWebThread::finishedThisRequest,
                              this, &QtWebServer::threadAvailable
                              );

            thread->start();
        }
    }

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
