#ifndef QTWEBSERVER_H
#define QTWEBSERVER_H

#include "qt-web-server_global.h"

#include <QtNetwork/QTcpServer>

class QtWebServerPrivate;

class QTWEBSERVERSHARED_EXPORT QtWebServer : public QTcpServer
{
        Q_OBJECT
    public:
        QtWebServer();

        void setSecure(bool isSecure);
        bool isSecure() const;

    protected:
        QtWebServerPrivate * d_ptr;

        virtual void incomingConnection(qintptr handle);

    private:
        Q_DECLARE_PRIVATE(QtWebServer)
};

#endif // QTWEBSERVER_H
