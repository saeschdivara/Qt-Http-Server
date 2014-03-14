#ifndef QTWEBTHREAD_H
#define QTWEBTHREAD_H

#include "qt-web-server_global.h"
#include <QtCore/QThread>

class QtWebThreadPrivate;

class QtWebThread : public QThread
{
        Q_OBJECT
    public:
        explicit QtWebThread(QObject *parent = 0);

        void setSocketHandle(qintptr handle);
        void setSecureSocket(bool isSecure);

    protected Q_SLOTS:
        void startHandlingConnection();
        void finishConnection(qint64 bytes);

    protected:
        QtWebThreadPrivate * d_ptr;

    private:
        Q_DECLARE_PRIVATE(QtWebThread)
};

#endif // QTWEBTHREAD_H
