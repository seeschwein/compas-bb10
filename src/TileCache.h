#ifndef TILECACHE_H
#define TILECACHE_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QStringBuilder>
#include <QtGui/QImage>
#include <QtSql/QSqlDatabase>

class TileCache : public QObject
{
    Q_OBJECT
public:
    explicit TileCache(const QString& dbPath, QObject* parent = 0);
    ~TileCache();

    /** Return a QImage if present in the cache (memory or disk). Returns a null QImage if missing. */
    QImage getImage(int z, int x, int y);

    /** Store an image (both in memory LRU and on disk). */
    void putImage(int z, int x, int y, const QImage& img);

    /** Optional: prune the on‑disk cache to stay under maxSizeBytes (LRU based on last access timestamp). */
    void pruneToSize(qint64 maxSizeBytes);

signals:
    void imageReady(int z, int x, int y);   // emitted when a newly fetched tile has been stored

private:
    QSqlDatabase db;
    QMutex mtx;

    // Simple in‑memory LRU: key = (z,x,y), value = QImage
    // We limit the memory cache to a reasonable number of entries (e.g. 64).
    QMap<QPair<int,int>, QImage> memCache;
    static const int MEM_CACHE_LIMIT = 64;

    void initDb();
    QString tilePath(int z, int x, int y) const;
};

#endif // TILECACHE_H