#ifndef QT_WEB_PRIVATE_RFC1036_H
#define QT_WEB_PRIVATE_RFC1036_H

#include <QtCore/QRegExp>
#include <QtCore/QDateTime>

class Rfc1036
{
    public:
        Rfc1036(const QByteArray &headerValue);

        operator bool();

        QDateTime toDateTime() const;

    private:
        static const QRegExp rfc1036;

        const QByteArray &headerValue;
        QRegExp regexp;
};

#endif // QT_WEB_PRIVATE_RFC1036_H
