#ifndef QTWEBTHREADCONTROLLER_H
#define QTWEBTHREADCONTROLLER_H

#include <QObject>

class QtWebThreadController : public QObject
{
        Q_OBJECT
    public:
        explicit QtWebThreadController(QObject *parent = 0);

    Q_SIGNALS:
        void connectionToHandle();
};

#endif // QTWEBTHREADCONTROLLER_H
