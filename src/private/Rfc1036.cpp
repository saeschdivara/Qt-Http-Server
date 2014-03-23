#include "Rfc1036.h"

const QRegExp Rfc1036::rfc1036("(?:\\w{3}\\s*,\\s*)" // day
                               "(\\d{1,2})\\s+" // day-1
                               "(\\w{3})\\s+" // month-2
                               "(\\d{2})\\s+" // year-3
                               "(\\d{2}):" // hour-4
                               "(\\d{2}):" // minutes-5
                               "(\\d{2})\\s+" // seconds-6
                               "GMT"
                               );

Rfc1036::Rfc1036(const QByteArray &headerValue) :
    headerValue(headerValue),
    regexp(rfc1036)
{}

QDateTime Rfc1036::toDateTime() const
{
    if (!regexp.captureCount())
        return QDateTime();

    int year = regexp.cap(3).toInt() + 1900, month = 1,
            day = regexp.cap(1).toInt();
    {
        QString monthStr(regexp.cap(2));
        if (monthStr == "Jan")
            month = 1;
        else if (monthStr == "Feb")
            month = 2;
        else if (monthStr == "Mar")
            month = 3;
        else if (monthStr == "Apr")
            month = 4;
        else if (monthStr == "May")
            month = 5;
        else if (monthStr == "Jun")
            month = 6;
        else if (monthStr == "Jul")
            month = 7;
        else if (monthStr == "Aug")
            month = 8;
        else if (monthStr == "Sep")
            month = 9;
        else if (monthStr == "Oct")
            month = 10;
        else if (monthStr == "Nov")
            month = 11;
        else if (monthStr == "Dec")
            month = 12;
    }

    int hours = regexp.cap(4).toInt(), minutes = regexp.cap(5).toInt(),
            seconds = regexp.cap(6).toInt();

    return QDateTime(QDate(year, month, day), QTime(hours, minutes, seconds),
                     Qt::UTC);
}

Rfc1036::operator bool()
{
    return regexp.indexIn(headerValue) != -1;
}

