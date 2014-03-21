#ifndef QTWEBSERVER_H
#define QTWEBSERVER_H

#include "qt-web-server_global.h"
#include "QtWebRequest.h"
#include "QtWebResponse.h"

#include <QtNetwork/QTcpServer>

class QtWebServerPrivate;

class QTWEBSERVERSHARED_EXPORT QtWebServer : public QTcpServer
{
        Q_OBJECT
    public:
        QtWebServer();
        virtual ~QtWebServer();

        void setSecure(bool isSecure);
        bool isSecure() const;

    Q_SIGNALS:
        void clientConnectionReady(QtWebRequest *, QtWebResponse *);

    protected:
        QtWebServerPrivate * d_ptr;

        virtual void incomingConnection(qintptr handle);

    private:
        Q_DECLARE_PRIVATE(QtWebServer)
};

#endif // QTWEBSERVER_H
