#include "WebAppServer.h"

#include <QtCore/QDir>
#include <QtCore/QThread>

QString realPath(const QString & path) {
    QString realPath;
    QStringList splittedPath = path.split(QDir::separator());
    for ( QString pathPart : splittedPath ) {
        if ( pathPart.endsWith(".app") ) break;
        realPath += pathPart + QDir::separator();
    }

    return realPath;
}

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

    qDebug() << request->requestPath();
    qDebug() << request->get();


    // Don't forget to have static-files in your output dir or at least a link of it
    if ( request->requestPath() == "/static/styles.css" ) {
        response->serveStaticFile(realPath(QDir::currentPath()) + "static-files", "/static", request);
    }
    else {
        QByteArray body("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"/static/styles.css\"></head>"
                        "<body><h1>Hello</h1></body></html>");

        response->setStatus(QtWebResponse::StatusCode::OK, "OK");
        response->write(body);
        response->end();
    }
}
