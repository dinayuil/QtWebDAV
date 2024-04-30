#ifndef QWEBDAVCLIENT_H
#define QWEBDAVCLIENT_H

#include <QObject>
#include "qwebdav_global.h"
#include "qwebdav.h"

class QWEBDAVSHARED_EXPORT QWebdavClient : public QObject
{
    Q_OBJECT
public:
    explicit QWebdavClient(
        const QWebdav::ConnectionType connectionType,
        const QString &hostname,
        const QString &rootPath = "/",
        const QString &username = "",
        const QString &password = "",
        int port = 0,
        const QString &sslCertDigestMd5 = "",
        const QString &sslCertDigestSha1 = "",
        QObject *parent = nullptr);

    bool check(QString path) const;

private:
    QWebdav* m_webdavManager;

signals:

};

#endif // QWEBDAVCLIENT_H
