#include "qwebdavclient.h"
#include <QDomDocument>
#include <iostream>
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
    // TODO delete reply?
    if(reply->error() == QNetworkReply::NoError) return true;
    return false;
}

bool QWebdavClient::upload(QString localFile, QString remoteFile) const
{
    QFile file(localFile);
    file.open(QIODevice::ReadOnly);
    QEventLoop loop;
    connect(m_webdavManager, &QWebdav::finished, &loop, &QEventLoop::quit); // TODO: time out?
    QNetworkReply* reply = m_webdavManager->put(remoteFile, &file);
    connect(reply, &QNetworkReply::uploadProgress, this, &QWebdavClient::uploadProgress);
    loop.exec();
    file.close();

    // TODO delete reply?
    if(reply->error() == QNetworkReply::NoError) return true;
    return false;
}

bool QWebdavClient::download(QString localFile, QString remoteFile) const
{
    QEventLoop loop;
    connect(m_webdavManager, &QWebdav::finished, &loop, &QEventLoop::quit); // TODO: time out?
    QNetworkReply* reply = m_webdavManager->get(remoteFile);
    connect(reply, &QNetworkReply::downloadProgress, this, &QWebdavClient::downloadProgress);
    loop.exec();

    if(reply->error() != QNetworkReply::NoError) return false;  // TODO log error?

    QByteArray data = reply->readAll();
#ifdef DEBUG_WEBDAV
    qDebug() << "downloaded: " << data;
#endif
    QFile file(localFile);
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
    // TODO delete reply?

    return true;
}

QStringList QWebdavClient::list(QString remoteDirectory) const
{
    QWebdav::PropNames props;
//    props["DAV:"] = QStringList();
//    props["DAV:"].append("resourcetype");

    QTimer timer;
    QEventLoop loop;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(m_webdavManager, &QWebdav::finished, &loop, &QEventLoop::quit);
    QNetworkReply* reply = m_webdavManager->propfind(remoteDirectory, props, 1);
    timer.start(10000);
    loop.exec();

    QDomDocument doc;
    doc.setContent(reply,QDomDocument::ParseOption::UseNamespaceProcessing);
    QDomElement multistatus = doc.elementsByTagName("multistatus").at(0).toElement();
    QDomNodeList responseElements = multistatus.elementsByTagName("response");
    QStringList fileNameList;
    for(int i = 0; i < responseElements.size(); i++)
    {
        QDomElement responseElement = responseElements.at(i).toElement();
        QDomElement hrefElement = responseElement.elementsByTagName("href").at(0).toElement();
        fileNameList.append(hrefElement.text());
    }

    return fileNameList;
}

void QWebdavClient::uploadProgress(qint64 bytesSent, qint64 bytesTotal) const
{
#ifdef DEBUG_WEBDAV
    qDebug() << "sent: " << bytesSent << ", total: " << bytesTotal;
#endif
}

void QWebdavClient::downloadProgress(qint64 bytesReceived, qint64 bytesTotal) const
{
#ifdef DEBUG_WEBDAV
    qDebug() << "receive: " << bytesReceived << ", total: " << bytesTotal;
#endif
}

