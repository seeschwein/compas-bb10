#ifndef TILENETWORK_H
#define TILENETWORK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class TileCache; // forward declaration

class TileNetwork : public QObject
{
    Q_OBJECT
public:
    explicit TileNetwork(TileCache* cache, QObject* parent = nullptr);
    ~TileNetwork();

    /** Trigger a fetch for tile (z,x,y) from the default OSM server */
    void fetchTile(int z, int x, int y);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    TileCache* m_cache;
    QNetworkAccessManager m_nam;
};

#endif // TILENETWORK_H