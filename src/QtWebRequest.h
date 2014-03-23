#ifndef QTWEBREQUEST_H
#define QTWEBREQUEST_H

#include "qt-web-server_global.h"

#include <QObject>
#include <QtCore/QByteArray>
#include <QtCore/QHash>

class QtWebRequestPrivate;

class QTWEBSERVERSHARED_EXPORT QtWebRequest : public QObject
{
        Q_OBJECT
    public:
        explicit QtWebRequest(QObject *parent = 0);
        virtual ~QtWebRequest();

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

        void setIP(const QString & ip);
        QString ip() const;

        void setMethod(RequestMethod method);
        RequestMethod method() const;

        void setRequestPath(const QByteArray & path);
        QByteArray requestPath() const;

        void setHttpVersion(HttpVersion version);
        HttpVersion httpVersion() const;

        void setHeaders(QHash<QByteArray, QByteArray> headers);
        bool containsHeader(QByteArray headerKey) const;
        QByteArray headerValue(QByteArray headerKey) const;
        QHash<QByteArray, QByteArray> headers() const;

        void insertPostValue(QByteArray key, QByteArray value);
        QHash<QByteArray, QByteArray> post() const;

        void insertPostFile(QByteArray key, QString fullFilePath);
        QHash<QByteArray, QString> files() const;

    protected:
        QtWebRequestPrivate * d_ptr;

    private:
        Q_DECLARE_PRIVATE(QtWebRequest)
};

#endif // QTWEBREQUEST_H
