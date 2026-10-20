#include "TileNetwork.h"
#include <QDebug>
#include <QNetworkReply>
#include <QUrl>

TileNetwork::TileNetwork(TileCache* cache, QObject* parent)
    : QObject(parent), m_cache(cache)
{
    // Optionally set a custom User-Agent so tile servers know who you are
    m_nam.setHeader(QNetworkRequest::UserAgentHeader,
                   "CompasBB10/1.0 (+https://github.com/yourname/compas-bb10)");
}

TileNetwork::~TileNetwork() = default;

void TileNetwork::fetchTile(int z, int x, int y)
{
    QString url = QStringLiteral("https://tile.openstreetmap.org/%1/%2/%3.png")
                  .arg(z).arg(x).arg(y);
    QNetworkRequest req(QUrl(url));

    // Store z,x,y as properties so we can retrieve them in the finished slot
    QNetworkReply* reply = m_nam.get(req);
    reply->setProperty("_z", z);
    reply->setProperty("_x", x);
    reply->setProperty("_y", y);
    connect(reply, &QNetworkReply::finished, this, &TileNetwork::onReplyFinished);
}

void TileNetwork::onReplyFinished(QNetworkReply* reply)
{
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