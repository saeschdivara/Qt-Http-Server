#include "QtWebThread.h"

#include <QtCore/QDebug>
#include <QtNetwork/QTcpSocket>

class QtWebThreadPrivate
{
    public:
        qintptr socketHandle;
        bool isUsingSecureConnections;

        QByteArray data;
        qint64 bytesWritten;
};

QtWebThread::QtWebThread(QObject *parent) :
    QThread(parent),
    d_ptr(new QtWebThreadPrivate)
{
    QObject::connect( this, &QtWebThread::started,
                      this, &QtWebThread::startHandlingConnection,
                      // This is needed so the slots is still in the same thread
                      Qt::DirectConnection
                      );
}

void QtWebThread::setSocketHandle(qintptr handle)
{
    Q_D(QtWebThread);

    d->socketHandle = handle;
}

void QtWebThread::setSecureSocket(bool isSecure)
{
    Q_D(QtWebThread);

    d->isUsingSecureConnections = isSecure;
}

void QtWebThread::startHandlingConnection()
{
    Q_D(QtWebThread);

    qDebug() << "startHandlingConnection" << currentThread();

    QTcpSocket * socket = new QTcpSocket;
    socket->setSocketDescriptor(d->socketHandle);

    QObject::connect( socket, &QTcpSocket::bytesWritten,
                      this, &QtWebThread::finishConnection
                      );

    const char data[] = "HTTP/1.1 202 OK\r\n";
    d->data.append(data);

    QByteArray body("<h1>Test</h1>");

    QByteArray header = "Content-Length; " + QByteArray::number(body.length()) + "\r\n";

    d->data.append(header);
    d->data.append("\r\n");
    d->data.append(body);
    d->data.append("\r\n");

    d->bytesWritten = 0;

    socket->write(d->data);
}

void QtWebThread::finishConnection(qint64 bytes)
{
    Q_D(QtWebThread);

    QTcpSocket * socket = qobject_cast<QTcpSocket *>(sender());

    d->bytesWritten += bytes;

    if ( d->bytesWritten == d->data.size() ) {
        socket->close();
    }
}
