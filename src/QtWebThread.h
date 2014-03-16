#ifndef QTWEBTHREAD_H
#define QTWEBTHREAD_H

#include "qt-web-server_global.h"

#include <QtCore/QThread>
#include <QtNetwork/QTcpSocket>

class QtWebThreadPrivate;

class QtWebThread : public QThread
{
        Q_OBJECT
    public:
        explicit QtWebThread(QObject *parent = 0);

        void setSocketHandle(qintptr handle);
        void setSecureSocket(bool isSecure);

    Q_SIGNALS:
        void everythingParsed();
        void finishedThisRequest();

    protected Q_SLOTS:
        void startHandlingConnection();
        void readyToRead();
        void readyToReadPostData();
        void parsePostData();
        void readyToWrite();
        void finishConnection(qint64 bytes);

    protected:
        QtWebThreadPrivate * d_ptr;

    private:
        Q_DECLARE_PRIVATE(QtWebThread)
};

#endif // QTWEBTHREAD_H
