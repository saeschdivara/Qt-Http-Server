#ifndef QT_WEB_PRIVATE_ASCTIME_H
#define QT_WEB_PRIVATE_ASCTIME_H

#include <QtCore/QRegExp>
#include <QtCore/QDateTime>

class Asctime
{
    public:
        Asctime(const QByteArray &headerValue);

        operator bool();

        QDateTime operator ()();

    private:
        static const QRegExp asctime;

        const QByteArray &headerValue;
        QRegExp regexp;
};

#endif // QT_WEB_PRIVATE_ASCTIME_H
