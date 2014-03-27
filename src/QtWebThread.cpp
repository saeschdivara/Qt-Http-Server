#include "QtWebThread.h"
#include "QtWebThreadController.h"
#include "QtWebRequest.h"
#include "QtWebResponse.h"

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtNetwork/QHostAddress>

class QtWebThreadPrivate
{
    public:
        QtWebThreadController * controller;

        qintptr socketHandle;
        bool isUsingSecureConnections;

        QTcpSocket * socket = Q_NULLPTR;

        QByteArray data;
        qint64 bytesWritten;

        QByteArray requestData;
        QByteArray postBoundary;

        QtWebRequest * request;
};

QtWebThread::QtWebThread(QObject *parent) :
    QThread(parent),
    d_ptr(new QtWebThreadPrivate)
{
    QObject::connect( this, &QtWebThread::started,
                      this, &QtWebThread::onStartUp,
                      // This is needed so the slots is still in the same thread
                      Qt::DirectConnection
                      );

    QObject::connect( this, &QtWebThread::everythingParsed,
                      this, &QtWebThread::readyToWrite,
                      Qt::DirectConnection
                      );

    setTerminationEnabled(true);
}

QtWebThread::~QtWebThread()
{
    delete d_ptr;
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

void QtWebThread::invokeNewPower()
{
    Q_EMIT restart();
}

void QtWebThread::onStartUp()
{
    Q_D(QtWebThread);

    d->controller = new QtWebThreadController;

    QObject::connect( this, &QtWebThread::restart,
                      d->controller, &QtWebThreadController::connectionToHandle,
                      Qt::QueuedConnection
                      );

    QObject::connect( d->controller, &QtWebThreadController::connectionToHandle,
                      this, &QtWebThread::startHandlingConnection,
                      Qt::DirectConnection
                      );

    startHandlingConnection();
}

void QtWebThread::startHandlingConnection()
{
    Q_D(QtWebThread);

    d->socket = new QTcpSocket;
    d->socket->setSocketDescriptor(d->socketHandle);

    QObject::connect( d->socket, &QTcpSocket::readyRead,
                      this, &QtWebThread::readyToRead,
                      Qt::DirectConnection
                      );
}

void QtWebThread::readyToRead()
{
    Q_D(QtWebThread);

    QObject::disconnect( d->socket, &QTcpSocket::readyRead,
                      this, &QtWebThread::readyToRead
                      );

    QObject::connect( d->socket, &QTcpSocket::readyRead,
                      this, &QtWebThread::readyToReadPostData,
                      Qt::DirectConnection
                      );

    QByteArray data = d->socket->readAll();

    QByteArray methodString = data.left(3);

    d->request = new QtWebRequest;
    d->request->setIP(d->socket->peerAddress().toString());

    QByteArray CRLF("\r\n");

    // Method
    QtWebRequest::RequestMethod method;

    if ( methodString == "GET" ) {
        method = QtWebRequest::RequestMethod::Get;
    }
    else if ( methodString == "POS" ) {
        method = QtWebRequest::RequestMethod::Post;
    }
    else if ( methodString == "HEA" ) {
        method = QtWebRequest::RequestMethod::Head;
    }
    else {
        method = QtWebRequest::RequestMethod::Unsupported;
    }

    d->request->setMethod(method);

    // Request path
    int indexOfRequestPathStart = data.indexOf("/");
    int indexOfRequestPathEnd = data.indexOf(" ", indexOfRequestPathStart);
    QByteArray requestPath = data.mid(indexOfRequestPathStart, indexOfRequestPathEnd - indexOfRequestPathStart);

    d->request->setRequestPath(requestPath);

    // Http version
    int indexOfBreak = data.indexOf(CRLF);
    QByteArray httpVersionString = data.mid(indexOfRequestPathEnd + 1, indexOfBreak - indexOfRequestPathEnd - 1);
    QtWebRequest::HttpVersion httpVersion = QtWebRequest::HttpVersion::Unknown;

    if ( httpVersionString == "HTTP/1.1" ) {
        httpVersion = QtWebRequest::HttpVersion::v1_1;
    }
    if ( httpVersionString == "HTTP/1.0" ) {
        httpVersion = QtWebRequest::HttpVersion::v1_0;
    }

    d->request->setHttpVersion(httpVersion);

    data = data.remove(0, indexOfBreak + CRLF.length());

    // Headers
    QHash<QByteArray, QByteArray> headers;

    Q_FOREVER {
        indexOfBreak = data.indexOf(CRLF);
        QByteArray headerLine = data.left(indexOfBreak);

        QByteArray key;
        QByteArray value;

        QList<QByteArray> splittedLine = headerLine.split(':');

        key = splittedLine.first();

        if ( splittedLine.length() >= 2 ) {

            value = splittedLine.at(1);
            value = value.right(value.length() - 1);

            for (int i = 2; i < splittedLine.length(); ++i) {
                value += ":" + splittedLine.at(i);
            }
        }
        else {
            value = "";
        }

        headers.insert(key, value);

        data = data.remove(0, indexOfBreak + CRLF.length());

        if ( data.startsWith(CRLF) ) {
            data = data.remove(0, CRLF.length());

            break;
        }
    }

    d->request->setHeaders(headers);

    QByteArray contentType = headers.value("Content-Type");
    QByteArray multiPart("multipart/form-data; ");
    int indexOfStart = contentType.indexOf(multiPart);

    if ( indexOfStart == 0 ) {
        d->requestData = data;
        d->postBoundary = contentType.mid(multiPart.length());

        if ( data.endsWith("--\r\n") ) {

            QObject::disconnect( d->socket, &QTcpSocket::readyRead,
                              this, &QtWebThread::readyToReadPostData
                              );

            parsePostData();

            Q_EMIT everythingParsed();
        }
    }
    else {

        QObject::disconnect( d->socket, &QTcpSocket::readyRead,
                          this, &QtWebThread::readyToReadPostData
                          );

        Q_EMIT everythingParsed();
    }
}

void QtWebThread::readyToReadPostData()
{
    Q_D(QtWebThread);

    QByteArray newData = d->socket->readAll();
    d->requestData += newData;

    int indexOfEnd = newData.indexOf("--\r\n");

    if ( indexOfEnd != -1 ) {

        parsePostData();

        Q_EMIT everythingParsed();
    }
}

void QtWebThread::parsePostData()
{
    Q_D(QtWebThread);

    QByteArray boundarySearchString("boundary=");
    QByteArray boundaryPart = d->postBoundary;
    auto indexOfBoundary = boundaryPart.indexOf(boundarySearchString);
    if ( indexOfBoundary == -1 ) return;
    QByteArray boundary = boundaryPart.remove(indexOfBoundary, boundarySearchString.length());

    QByteArray start_end("--");
    QByteArray line_break("\r\n");
    QByteArray contentDisposition("Content-Disposition: ");
    QByteArray contentType("Content-Type: ");
    QByteArray name(" name=");
    QByteArray fileName(" filename=");

    QByteArray bodyData = d->requestData;

    Q_FOREVER {
        auto indexOfStart = bodyData.indexOf(start_end);
        if ( indexOfStart == -1 ) break;
        bodyData = bodyData.remove(indexOfStart, start_end.length());

        auto indexOfBoundary = bodyData.indexOf(boundary);
        if ( indexOfBoundary == -1 ) break;
        bodyData = bodyData.remove(indexOfBoundary, boundary.length());

        auto indexOfBreak = bodyData.indexOf(line_break);
        if ( indexOfBreak == -1 ) break;
        bodyData = bodyData.remove(indexOfBreak, line_break.length());

        auto indexOfContentDisposition = bodyData.indexOf(contentDisposition);
        if ( indexOfContentDisposition == -1 ) break;
        bodyData = bodyData.remove(indexOfContentDisposition, contentDisposition.length());

        indexOfBreak = bodyData.indexOf(line_break);
        QByteArray contentDispositionLine = bodyData.left(indexOfBreak);
        QList<QByteArray> splittedDisposition = contentDispositionLine.split(';');
        bodyData = bodyData.remove(0, indexOfBreak + line_break.length());

        QByteArray namePart = splittedDisposition.at(1);

        auto indexOfName = namePart.indexOf(name);
        if ( indexOfName == -1 ) break;
        namePart = namePart.remove(indexOfName, name.length());

        QByteArray nameValue = namePart.left(namePart.length() -1);
        nameValue = nameValue.right(nameValue.length() -1);

        if ( splittedDisposition.size() == 2 ) {

            auto indexOfBreak = bodyData.indexOf(line_break);
            if ( indexOfBreak == -1 ) break;
            bodyData = bodyData.remove(indexOfBreak, line_break.length());

            indexOfBreak = bodyData.indexOf(line_break);
            if ( indexOfBreak == -1 ) break;
            QByteArray value = bodyData.mid(0, indexOfBreak);
            bodyData = bodyData.remove(0, indexOfBreak + line_break.length());

            d->request->insertPostValue(nameValue, value);
        }
        else {
            QByteArray fileNamePart = splittedDisposition.at(2);

            auto indexOfFileName = fileNamePart.indexOf(fileName);
            if ( indexOfFileName == -1 ) break;
            fileNamePart = fileNamePart.remove(indexOfFileName, fileName.length());

            QByteArray fileNameValue = fileNamePart.left(fileNamePart.length() -1);
            fileNameValue = fileNameValue.right(fileNameValue.length() -1);

            // In case anybody sends an empty file
            if ( fileNameValue.isEmpty() ) {
                QByteArray emptyFile = line_break + line_break + line_break;
                auto indexOfNextStart = bodyData.indexOf(emptyFile);
                bodyData = bodyData.remove(0, indexOfNextStart + emptyFile.size());
                continue;
            }

            auto indexOfContentType = bodyData.indexOf(contentType);
            if ( indexOfContentType == -1 ) break;
            bodyData = bodyData.remove(indexOfContentType, contentType.length());

            indexOfBreak = bodyData.indexOf(line_break);
            if ( indexOfBreak == -1 ) break;
            QByteArray mimeType = bodyData.left(indexOfBreak);
            bodyData = bodyData.remove(0, indexOfBreak + line_break.length());

            indexOfBreak = bodyData.indexOf(line_break);
            if ( indexOfBreak == -1 ) break;
            bodyData = bodyData.remove(indexOfBreak, line_break.length());

            // Create unique path so file names don't matter
            QDateTime now = QDateTime::currentDateTimeUtc();
            QString pathNowAddition = now.toString("ddMMyyyy_hhmmss.zzz");
            QString uploadPath = "./file_upload/" + pathNowAddition;

            QDir dir = QDir::current();
            dir.mkpath(uploadPath);
            QString cleanUploadPath = QDir::cleanPath(dir.absoluteFilePath(uploadPath));
            QString filePath = cleanUploadPath + QDir::separator() + fileNameValue;

            QFile file(filePath);
            if ( file.open(QIODevice::WriteOnly | QIODevice::Unbuffered) ) {

                const int space = 10000;

                QByteArray data;
                data.reserve(space);
                indexOfBreak = -1;

                while ( indexOfBreak == -1 ) {
                    data = bodyData.left(space);

                    indexOfBreak = data.indexOf("--\r\n");

                    if ( indexOfBreak == -1 ) {
                        bodyData = bodyData.remove(0, space);
                        file.write(data);
                    }
                    else {
                        data = bodyData.left(indexOfBreak);
                        bodyData = bodyData.remove(0, indexOfBreak);

                        file.write(data);
                    }

                    QThread::yieldCurrentThread();
                }

                file.close();

                d->request->insertPostValue(nameValue, filePath);
                d->request->insertPostFile(nameValue, filePath);
            }
        }
    }
}

void QtWebThread::readyToWrite()
{
    Q_D(QtWebThread);

    QtWebRequest * request = d->request;
    QTcpSocket * socket = d->socket;
    QtWebResponse * response = new QtWebResponse(socket);

    QObject::connect( response, &QtWebResponse::finishedConnection,
                      this, &QtWebThread::finishedThisRequest,
                      Qt::DirectConnection
                      );

    Q_EMIT clientConnectionReady(request, response);
}
