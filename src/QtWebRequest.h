#ifndef QTWEBREQUEST_H
#define QTWEBREQUEST_H

#include <QObject>
#include <QtCore/QByteArray>
#include <QtCore/QHash>

class QtWebRequestPrivate;

class QtWebRequest : public QObject
{
        Q_OBJECT
    public:
        explicit QtWebRequest(QObject *parent = 0);

        enum class RequestMethod {
            Unsupported = 0,
            Get = 1,
            Post = 2,
            Head = 3
        };

        enum class HttpVersion {
            Unknown = 0,
            v1_0 = 1,
            v1_1 = 2
        };

        void setMethod(RequestMethod method);
        RequestMethod method() const;

        void setRequestPath(const QByteArray & path);
        QByteArray requestPath() const;

        void setHttpVersion(HttpVersion version);
        HttpVersion httpVersion() const;

        void setHeaders(QHash<QByteArray, QByteArray> headers);
        QHash<QByteArray, QByteArray> headers() const;

        void insertPostValue(QByteArray key, QByteArray value);
        QHash<QByteArray, QByteArray> post() const;

    protected:
        QtWebRequestPrivate * d_ptr;

    private:
        Q_DECLARE_PRIVATE(QtWebRequest)
};

#endif // QTWEBREQUEST_H
