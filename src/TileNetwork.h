#ifndef TILENETWORK_H
#define TILENETWORK_H

#include <QtCore/QObject>
#include <QtNetwork/QNetworkAccessManager>

class TileCache; // forward declaration

class TileNetwork : public QObject
{
    Q_OBJECT
public:
    explicit TileNetwork(TileCache* cache, QObject* parent = 0);
    ~TileNetwork();

    /** Trigger a fetch for tile (z,x,y) */
    void fetchTile(int z, int x, int y);

private slots:
    void onReplyFinished();

private:
    TileCache* m_cache;
    QNetworkAccessManager m_nam;
};

#endif // TILENETWORK_H