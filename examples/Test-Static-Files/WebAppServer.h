#ifndef WEBAPPSERVER_H
#define WEBAPPSERVER_H

#include <QObject>
#include <QtWebRequest.h>
#include <QtWebResponse.h>

class WebAppServerPrivate;

class WebAppServer : public QObject
{
        Q_OBJECT
    public:
        explicit WebAppServer(QObject *parent = 0);

        void start();

    public Q_SLOTS:
        void handleConnection(QtWebRequest *request, QtWebResponse *response);

    protected:
        WebAppServerPrivate * d_ptr;

    private:
        Q_DECLARE_PRIVATE(WebAppServer)
};

#endif // WEBAPPSERVER_H
