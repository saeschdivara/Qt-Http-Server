#include "QtWebRequest.h"

class QtWebRequestPrivate
{
    public:
        QtWebRequest::RequestMethod method;
        QByteArray requestPath;
        QtWebRequest::HttpVersion httpVersion;
        QHash<QByteArray, QByteArray> headers;

        QHash<QByteArray, QByteArray> post;
        QHash<QByteArray, QString> postFiles;
        QHash<QByteArray, QByteArray> get;
};

QtWebRequest::QtWebRequest(QObject *parent) :
    QObject(parent),
    d_ptr(new QtWebRequestPrivate)
{
}

QtWebRequest::~QtWebRequest()
{
    delete d_ptr;
}

void QtWebRequest::setMethod(QtWebRequest::RequestMethod method)
{
    Q_D(QtWebRequest);

    d->method = method;
}

QtWebRequest::RequestMethod QtWebRequest::method() const
{
    Q_D(const QtWebRequest);

    return d->method;
}

void QtWebRequest::setRequestPath(const QByteArray &path)
{
    Q_D(QtWebRequest);

    d->requestPath = path;
}

QByteArray QtWebRequest::requestPath() const
{
    Q_D(const QtWebRequest);

    return d->requestPath;
}

void QtWebRequest::setHttpVersion(QtWebRequest::HttpVersion version)
{
    Q_D(QtWebRequest);

    d->httpVersion = version;
}

QtWebRequest::HttpVersion QtWebRequest::httpVersion() const
{
    Q_D(const QtWebRequest);

    return d->httpVersion;
}

void QtWebRequest::setHeaders(QHash<QByteArray, QByteArray> headers)
{
    Q_D(QtWebRequest);

    d->headers = headers;
}

QHash<QByteArray, QByteArray> QtWebRequest::headers() const
{
    Q_D(const QtWebRequest);

    return d->headers;
}

void QtWebRequest::insertPostValue(QByteArray key, QByteArray value)
{
    Q_D(QtWebRequest);

    d->post.insert(key, value);
}

QHash<QByteArray, QByteArray> QtWebRequest::post() const
{
    Q_D(const QtWebRequest);

    return d->post;
}

void QtWebRequest::insertPostFile(QByteArray key, QString fullFilePath)
{
    Q_D(QtWebRequest);

    d->postFiles.insert(key, fullFilePath);
}

QHash<QByteArray, QString> QtWebRequest::files() const
{
    Q_D(const QtWebRequest);

    return d->postFiles;
}
