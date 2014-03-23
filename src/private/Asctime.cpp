#include "Asctime.h"

const QRegExp Asctime::asctime("(?:\\w{3})\\s+" // day
                               "(\\w{3})\\s+" // month-1
                               "(\\d{1,2})\\s+" // day-2
                               "(\\d{2}):" // hour-3
                               "(\\d{2}):" // minutes-4
                               "(\\d{2})\\s+" // seconds-5
                               "(\\d{4})" // year-6
                               );
Asctime::Asctime(const QByteArray &headerValue) :
    headerValue(headerValue),
    regexp(asctime)
{}

Asctime::operator bool()
{
    return regexp.indexIn(headerValue) != -1;
}

QDateTime Asctime::operator ()()
{
    if (!regexp.captureCount())
        return QDateTime();

    int year = regexp.cap(6).toInt(), month = 1,
            day = regexp.cap(2).toInt();
    {
        QString monthStr(regexp.cap(1));
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

    int hours = regexp.cap(3).toInt(), minutes = regexp.cap(4).toInt(),
            seconds = regexp.cap(5).toInt();

    return QDateTime(QDate(year, month, day), QTime(hours, minutes, seconds),
                     Qt::UTC);
}
