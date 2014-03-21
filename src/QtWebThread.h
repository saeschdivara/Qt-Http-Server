#ifndef QTWEBTHREAD_H
#define QTWEBTHREAD_H

#include "qt-web-server_global.h"

#include "QtWebRequest.h"
#include "QtWebResponse.h"

#include <QtCore/QThread>
#include <QtNetwork/QTcpSocket>

class QtWebThreadPrivate;

class QTWEBSERVERSHARED_EXPORT QtWebThread : public QThread
{
        Q_OBJECT
    public:
        explicit QtWebThread(QObject *parent = 0);
        virtual ~QtWebThread();

        void setSocketHandle(qintptr handle);
        void setSecureSocket(bool isSecure);

    Q_SIGNALS:
        void everythingParsed();
        void clientConnectionReady(QtWebRequest *, QtWebResponse *);
        void finishedThisRequest();

    protected Q_SLOTS:
        void startHandlingConnection();
        void readyToRead();
        void readyToReadPostData();
        void parsePostData();
        void readyToWrite();

    protected:
        QtWebThreadPrivate * d_ptr;

    private:
        Q_DECLARE_PRIVATE(QtWebThread)
};

#endif // QTWEBTHREAD_H
