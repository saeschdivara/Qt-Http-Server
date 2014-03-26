#include "QtWebResponse.h"

#include "private/Asctime.h"
#include "private/Rfc1036.h"
#include "private/Rfc1123.h"

#include <QtCore/QByteArray>
#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QThread>

#define CRLF "\r\n"

QByteArray QtWebResponse::Helper::fromDateTime(const QDateTime &dateTime)
{
    return QLocale(QLocale::C).toString(dateTime.toUTC(),
                                        "ddd," // day
                                        " d MMM yyyy" // date
                                        " hh:mm:ss" // hour
                                        ).toUtf8() + " GMT"; // zone
}

QDateTime QtWebResponse::Helper::toDateTime(const QByteArray &headerValue,
                                            const QDateTime &defaultValue)
{
    {
        Rfc1123 rfc1123(headerValue);
        if (rfc1123)
            return rfc1123();
    }
    {
        Rfc1036 rfc1036(headerValue);
        if (rfc1036)
            return rfc1036.toDateTime();
    }
    {
        Asctime asctime(headerValue);
        if (asctime)
            return asctime();
    }

    return defaultValue;
}

static qint64 bufferSize = 1024 * 100;

inline static QList< QPair<qulonglong, qulonglong> >
ranges(const QHash<QByteArray, QByteArray> &headers, qulonglong fileSize)
{
    if (!headers.contains("Range"))
        return QList< QPair<qulonglong, qulonglong> >();

    QList< QPair<qulonglong, qulonglong> > ranges;

    QList<QByteArray> rangeHeaders(headers.values("Range"));
    for (QByteArray rangesSpecifier : rangeHeaders) {
        static const QByteArray bytesUnit("bytes=");
        if (!rangesSpecifier.startsWith(bytesUnit))
            continue;

        QList<QByteArray> rangeSet(rangesSpecifier.mid(bytesUnit.size())
                                   .split(','));
        for (QByteArray rangeSpec : rangeSet) {
            rangeSpec = rangeSpec.trimmed();

            if (rangeSpec.startsWith('-')) {
                bool ok;
                qulonglong nbytes = rangeSpec.mid(1).toULongLong(&ok);
                if (!ok || nbytes == 0)
                    continue;

                if (nbytes >= fileSize) {
                    ranges.push_back(QPair<qulonglong, qulonglong>
                                     (0, fileSize - 1));
                } else {
                    ranges.push_back(QPair<qulonglong, qulonglong>
                                     (fileSize - nbytes, fileSize - 1));
                }
            } else {
                int i = rangeSpec.indexOf('-');
                if (i == -1)
                    continue;

                bool ok[2];
                QPair<qulonglong, qulonglong>
                        range(rangeSpec.toULongLong(ok),
                              rangeSpec.mid(1 + i).toULongLong());

                if (!ok[1])
                    range.second = fileSize - 1;

                if (!ok[0] || range.second < range.first
                        || range.first >= fileSize)
                    continue;

                if (range.second >= fileSize)
                    range.second = fileSize;

                ranges.push_back(range);
            }
        }
    }

    return ranges;
}

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

        qint64 dataSize = -1;
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

void QtWebResponse::serveStaticFile(const QString &dir,
                                    const QString & staticPath,
                                    QtWebRequest *request)
{
    Q_D(QtWebResponse);

    QString fileName;

    QByteArray encoded = QByteArray::fromPercentEncoding(request->requestPath());
    encoded = encoded.replace(staticPath, "");

    fileName = dir + encoded;

    // Check if method is alright
    {
        QtWebRequest::RequestMethod method = request->method();
        if (
                method != QtWebRequest::RequestMethod::Get
                &&
                method != QtWebRequest::RequestMethod::Head
                ) {
            setStatus(StatusCode::METHOD_NOT_ALLOWED, "");
            addHeader("Allow", "GET, HEAD");
            end();
            return;
        }
    }

    QFileInfo fileInfo(fileName);

    if (!fileInfo.exists()) {
        setStatus(StatusCode::NOT_FOUND);
        end();
        return;
    }

    // Check "If-Modified-Since" header
    if (request->containsHeader("If-Modified-Since")) {
        const QByteArray &value = request->headerValue("If-Modified-Since");
        QDateTime date(Helper::toDateTime(value));

        // Ignore invalid values
        if (fileInfo.lastModified() < date || !date.isValid()) {
            //continue;
        } else if (fileInfo.lastModified() == date) {
            setStatus(StatusCode::NOT_MODIFIED);
            end();
        }
    }

    // Check "If-Unmodified-Since" header
    if (request->containsHeader("If-Unmodified-Since")) {
        const QByteArray &value = request->headerValue("If-Unmodified-Since");
        QDateTime date(Helper::toDateTime(value));

        // Ignore invalid values
        if (fileInfo.lastModified() < date || !date.isValid()) {
            //continue;
        } else if (fileInfo.lastModified() > date) {
            setStatus(StatusCode::PRECONDITION_FAILED);
            end();
        }
    }


    // Check If-Range header
    if (request->headers().contains("If-Range")
            && request->headers().contains("Range")) {
        foreach (QByteArray value, request->headers().values("If-Range")) {
            // Ignore ETags
            int strSize = value.size();
            if ((strSize > 0 && value[0] == '"')
                    || (strSize > 1 && value[0] == 'W' && value[1] == '/')) {
                continue;
            } else {
                QDateTime date(Helper::toDateTime(value));

                // Ignore invalid values
                if (fileInfo.lastModified() < date || !date.isValid()) {
                    continue;
                } else if (fileInfo.lastModified() > date) {
                    // Return the entire entity using a 200 response
                    request->headers().remove("Range");
                    break;
                }
            }
        }
    }

    // All conditionals were okay, continue...

    addHeader("Accept-Ranges", "bytes");
    addHeader("Date", Helper::fromDateTime(QDateTime::currentDateTime()));
    addHeader("Last-Modified", Helper::fromDateTime(fileInfo.lastModified()));

    if (request->method() == QtWebRequest::RequestMethod::Head) {
        setStatus(StatusCode::OK);
        end();
        return;
    }

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    QList< QPair<qulonglong, qulonglong> >
            ranges(::ranges(request->headers(), fileInfo.size()));

    // Not a byterange request
    if (!ranges.size()) {
        // Not a _satisfiable_ byterange request
        if (request->headers().contains("Range")) {
            static const QByteArray bytesUnit("bytes */");

            setStatus(StatusCode::REQUESTED_RANGE_NOT_SATISFIABLE);
            addHeader("Content-Range", bytesUnit + QByteArray::number(fileInfo.size()));
            end();
            return;
        }

        // Just send the entity
        setStatus(StatusCode::OK);
        writeHeader();

        d->dataSize += file.size();

        while (!file.atEnd()) {
            QByteArray data = file.read(::bufferSize);
            d->socket->write(data);
            d->socket->flush();
        }

        endFile();
    } else if (ranges.size() == 1) {
        // ONE range
        static const QByteArray bytesUnit("bytes ");

        setStatus(StatusCode::OK);
        QPair<qulonglong, qulonglong> &range(ranges[0]);
        addHeader("Content-Range", bytesUnit
                  + QByteArray::number(range.first)
                  + '-'
                  + QByteArray::number(range.second)
                  + '/'
                  + QByteArray::number(fileInfo.size()));

        file.seek(range.first);

        qint64 remaining = 1 + range.second - range.first;
        while (remaining) {
            QByteArray chunk(file.read(qMin(remaining, ::bufferSize)));
            d->socket << chunk;
            d->socket->flush();
            remaining -= chunk.size();
        }

        end();
    } else {
        // MULTIPLE ranges
        QByteArray contentType;
        int i = 0;

        for ( QPair<QByteArray, QByteArray> pair : d->headers ) {
            if ( pair.first == "Content-Type" ) {
                contentType = pair.second;
                break;
            }

            i++;
        }

        d->headers.replace(i,
                           qMakePair(QByteArray("Content-Type"),
                                     QByteArray("multipart/byteranges;"
                                                "boundary=THIS_STRING_SEPARATES")
                                     )
                           );

        for (int i = 0;i != ranges.size();++i) {
            QPair<qulonglong, qulonglong> &range(ranges[i]);
            d->socket << QByteArray("--THIS_STRING_SEPARATES\r\n");
            if (!contentType.isEmpty())
                d->socket << QByteArray("Content-type: ") << contentType << "\r\n";

            d->socket << QByteArray("Content-range: bytes")
                      << QByteArray::number(range.first) << QByteArray(1, '-')
                      << QByteArray::number(range.second)
                      << QByteArray(1, '/')
                      << QByteArray::number(fileInfo.size()) << "\r\n"
                      << QByteArray("\r\n");

            file.seek(range.first);

            qint64 remaining = 1 + range.second - range.first;
            while (remaining) {
                QByteArray chunk(file.read(qMin(remaining, ::bufferSize)));
                d->socket << chunk;
                d->socket->flush();
                remaining -= chunk.size();
            }
        }
        write("--THIS_STRING_SEPARATES--\r\n");
        end();
    }
}

void QtWebResponse::setStatus(QtWebResponse::StatusCode code)
{
    setStatus(code, QByteArray(""));
}

void QtWebResponse::setStatus(QtWebResponse::StatusCode code, const QByteArray & reasonPhrase)
{
    Q_D(QtWebResponse);

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

    d->bodyData += data;
}

void QtWebResponse::end()
{
    Q_D(QtWebResponse);

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

    d->bytesWritten += bytes;

    bool isFinished =
            (d->bytesWritten == d->dataSize  && d->dataSize > 0)
            ||
            (d->bytesWritten >= d->data.size() && d->data.size() > 0);

    qDebug() << d->bytesWritten << "/" << d->dataSize << "(" << isFinished << ")";

    if ( isFinished ) {

        d->socket->close();

        Q_EMIT finishedConnection();
    }
}

void QtWebResponse::writeHeader()
{
    Q_D(QtWebResponse);


    d->bytesWritten = 0;
    d->dataSize = 0;
    QByteArray headerData;

    QByteArray start = "HTTP/1.1 " + QByteArray::number(int(d->status)) + " " + d->reason + CRLF;
    headerData.append(start);

    QByteArray headers = "Content-Length; " + QByteArray::number(d->bodyData.length()) + CRLF;

    for ( QPair<QByteArray, QByteArray> headerPair : d->headers ) {
        QByteArray header;

        header += headerPair.first;
        header += ": ";
        header += headerPair.second;
        header += CRLF;

        headers += header;
    }

    headerData.append(headers);
    headerData.append(CRLF);

    d->dataSize += headerData.size();

    d->socket->write(headerData);
}

void QtWebResponse::endFile()
{
    Q_D(QtWebResponse);

    d->dataSize += sizeof(CRLF);
    d->socket->write(CRLF, sizeof(CRLF));
}

