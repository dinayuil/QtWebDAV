#include "qwebdavclient.h"

QWebdavClient::QWebdavClient(
    const QWebdav::ConnectionType connectionType,
    const QString &hostname,
    const QString &rootPath,
    const QString &username,
    const QString &password,
    int port,
    const QString &sslCertDigestMd5,
    const QString &sslCertDigestSha1,
    QObject *parent)
    : QObject{parent}
{
    m_webdavManager = new QWebdav(parent);
    m_webdavManager->setConnectionSettings(
        connectionType,
        hostname,
        rootPath,
        username,
        password,
        port,
        sslCertDigestMd5,
        sslCertDigestSha1
        );
}

bool QWebdavClient::check(QString path) const
{
    QWebdav::PropNames props;
    props["DAV:"] = QStringList();
    props["DAV:"].append("resourcetype");

    QTimer timer;
    QEventLoop loop;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(m_webdavManager, &QWebdav::finished, &loop, &QEventLoop::quit);
    QNetworkReply* reply = m_webdavManager->propfind(path, props, 0);
    timer.start(10000);
    loop.exec();

#ifdef DEBUG_WEBDAV
    while(reply->canReadLine())
    {
        qDebug() << reply->readLine();
    }
#endif

    if(reply->error() == QNetworkReply::NoError) return true;
    return false;
}
