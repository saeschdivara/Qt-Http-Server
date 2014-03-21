#include "WebAppServer.h"

#include <QtCore/QThread>

class WebAppServerPrivate
{
    public:
};

WebAppServer::WebAppServer(QObject *parent) :
    QObject(parent),
    d_ptr(new WebAppServerPrivate)
{
}

void WebAppServer::handleConnection(QtWebRequest * request, QtWebResponse * response)
{
    qDebug() << QThread::currentThread() << "handleConnection";

    QByteArray body("<html><body><form action=\".\" method=\"post\" enctype=\"multipart/form-data\">"
                    "<input name=\"file_dd\" type=\"file\" /><input type=\"submit\" />"
                    "</form></body></html>");

    response->setStatus(QtWebResponse::StatusCode::OK, "OK");
    response->write(body);
    response->end();
}
