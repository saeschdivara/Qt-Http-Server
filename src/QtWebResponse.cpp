#include "QtWebResponse.h"

#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QThread>

#define CRLF "\r\n"

class QtWebResponsePrivate
{
    public:
        QTcpSocket * socket;

        QByteArray bodyData;
        QByteArray data;
        qint64 bytesWritten;

        QtWebResponse::StatusCode status;
        QByteArray reason;

        QList< QPair<QByteArray, QByteArray> > headers;
};

QtWebResponse::QtWebResponse(QTcpSocket *socket, QObject *parent) :
    QObject(parent),
    d_ptr(new QtWebResponsePrivate)
{
    Q_D(QtWebResponse);

    d->socket = socket;

    QObject::connect( socket, &QTcpSocket::bytesWritten,
                      this, &QtWebResponse::finishConnection
                      );
}

QtWebResponse::~QtWebResponse()
{
    delete d_ptr;
}

void QtWebResponse::setStatus(QtWebResponse::StatusCode code, const QByteArray & reasonPhrase)
{
    Q_D(QtWebResponse);

    qDebug() << QThread::currentThread() << "setStatus";

    d->status = code;
    d->reason = reasonPhrase;
}

void QtWebResponse::addHeader(const QByteArray &key, const QByteArray &value)
{
    Q_D(QtWebResponse);

    d->headers << qMakePair(key, value);
}

void QtWebResponse::write(const QByteArray &data)
{
    Q_D(QtWebResponse);

    qDebug() << QThread::currentThread() << "write";

    d->bodyData += data;
}

void QtWebResponse::end()
{
    Q_D(QtWebResponse);

    qDebug() << QThread::currentThread() << "end";

    QByteArray start = "HTTP/1.1 " + QByteArray::number(int(d->status)) + " " + d->reason + CRLF;
    d->data.append(start);

    QByteArray headers = "Content-Length; " + QByteArray::number(d->bodyData.length()) + CRLF;

    for ( QPair<QByteArray, QByteArray> headerPair : d->headers ) {
        QByteArray header;

        header += headerPair.first;
        header += ": ";
        header += headerPair.second;
        header += CRLF;

        headers += header;
    }

    d->data.append(headers);
    d->data.append(CRLF);
    d->data.append(d->bodyData);
    d->data.append(CRLF);

    d->bytesWritten = 0;

    d->socket->write(d->data);
}

void QtWebResponse::finishConnection(qint64 bytes)
{
    Q_D(QtWebResponse);

    qDebug() << QThread::currentThread() << "finishConnection";

    d->bytesWritten += bytes;

    qDebug() << d->bytesWritten << "/" << d->data.size();

    if ( d->bytesWritten == d->data.size() ) {
        d->socket->close();

        Q_EMIT finishedConnection();
    }
}
