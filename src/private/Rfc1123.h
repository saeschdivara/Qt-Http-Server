#ifndef QT_WEB_PRIVATE_RFC1123_H
#define QT_WEB_PRIVATE_RFC1123_H

#include <QtCore/QRegExp>
#include <QtCore/QDateTime>

class Rfc1123
{
    public:
        Rfc1123(const QByteArray &headerValue);

        operator bool();

        QDateTime operator ()();

    private:
        static const QRegExp rfc1123;

        const QByteArray &headerValue;
        QRegExp regexp;
};

#endif // QT_WEB_PRIVATE_RFC1123_H
