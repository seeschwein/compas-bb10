#include "TileNetwork.h"
#include "TileCache.h"
#include <QtCore/QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QUrl>
#include <QtGui/QImage>

TileNetwork::TileNetwork(TileCache* cache, QObject* parent)
    : QObject(parent), m_cache(cache), m_nam(this)
{
    // Qt 4.8.6: User-Agent is set via setRawHeader on the request, not on the manager
    // We'll set it per-request in fetchTile
}

TileNetwork::~TileNetwork() = default;

void TileNetwork::fetchTile(int z, int x, int y)
{
    // Qt 4.8.6: use QString for string formatting
    QString url = QString("https://tile.openstreetmap.org/%1/%2/%3.png")
                  .arg(z).arg(x).arg(y);
    QNetworkRequest req((QUrl(url)));
    req.setRawHeader(QByteArray("User-Agent"),
                     QByteArray("CompasBB10/1.0 (+https://github.com/yourname/compas-bb10)"));

    // Store z,x,y as properties so we can retrieve them in the finished slot
    QNetworkReply* reply = m_nam.get(req);
    reply->setProperty("_z", z);
    reply->setProperty("_x", x);
    reply->setProperty("_y", y);

    // Qt 4.8.6: string-based connect (finished is protected in Qt 4)
    connect(reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
}

void TileNetwork::onReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "TileNetwork: error fetching tile:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QImage img;
    if (!img.loadFromData(data)) {
        qWarning() << "TileNetwork: failed to decode image data";
        reply->deleteLater();
        return;
    }

    int z = reply->property("_z").toInt();
    int x = reply->property("_x").toInt();
    int y = reply->property("_y").toInt();
    if (m_cache) {
        m_cache->putImage(z, x, y, img);
    }
    reply->deleteLater();
}