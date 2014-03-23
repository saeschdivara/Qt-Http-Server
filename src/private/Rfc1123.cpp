#include "Rfc1123.h"

const QRegExp Rfc1123::rfc1123("(?:\\w{3}\\s*,\\s*)?" // day
                               "(\\d{1,2})\\s+" // day-1
                               "(\\w{3})\\s+" // month-2
                               "(\\d{2}(?:\\d{2})?)\\s+" // year-3
                               "(\\d{2}):" // hour-4
                               "(\\d{2})" // minutes-5
                               "(?::(\\d{2}))?\\s*" // seconds-6
                               "GMT"
                               );

Rfc1123::Rfc1123(const QByteArray &headerValue) :
    headerValue(headerValue),
    regexp(rfc1123)
{}

Rfc1123::operator bool()
{
    return regexp.indexIn(headerValue) != -1;
}

QDateTime Rfc1123::operator ()()
{
    if (!regexp.captureCount())
        return QDateTime();

    int year, month = 1, day = regexp.cap(1).toInt();
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
    {
        QString yearStr(regexp.cap(3));
        if (yearStr.size() == 2)
            year = yearStr.toInt() + 1900;
        else
            year = yearStr.toInt();
    }

    int hours = regexp.cap(4).toInt(), minutes = regexp.cap(5).toInt(),
            seconds = 0;
    {
        QString secondsStr(regexp.cap(6));
        if (!secondsStr.isEmpty())
            seconds = secondsStr.toInt();
    }

    return QDateTime(QDate(year, month, day), QTime(hours, minutes, seconds),
                     Qt::UTC);
}
