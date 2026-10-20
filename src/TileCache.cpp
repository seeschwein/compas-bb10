#include "TileCache.h"
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

TileCache::TileCache(const QString& dbPath, QObject* parent)
    : QObject(parent)
{
    QFileInfo fi(dbPath);
    QDir().mkpath(fi.absolutePath());

    db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        qWarning() << "TileCache: cannot open DB:" << db.lastError().text();
    } else {
        initDb();
    }
}

TileCache::~TileCache()
{
    if (db.isOpen())
        db.close();
}

void TileCache::initDb()
{
    QSqlQuery q(db);
    if (!q.exec(QLatin1String(
        "CREATE TABLE IF NOT EXISTS tiles ("
        "z INTEGER NOT NULL,"
        "x INTEGER NOT NULL,"
        "y INTEGER NOT NULL,"
        "data BLOB NOT NULL,"
        "PRIMARY KEY (z, x, y)"
        ")")))
    {
        qWarning() << "TileCache: table creation failed:" << q.lastError().text();
    }
    if (!q.exec(QLatin1String("CREATE INDEX IF NOT EXISTS idx_zxy ON tiles(z,x,y);"))) {
        qWarning() << "TileCache: index creation failed:" << q.lastError().text();
    }
}

/* -----------------------------------------------------------------
   Helper: build a cache‑key that works with QMap<QPair<...>>
   ----------------------------------------------------------------- */
static QPair<int, QPair<int,int>> makeKey(int z, int x, int y)
{
    return qMakePair(z, qMakePair(x, y));
}

/* -----------------------------------------------------------------
   Public API – getImage
   ----------------------------------------------------------------- */
QImage TileCache::getImage(int z, int x, int y)
{
    QMutexLocker locker(&mtx);
    auto key = makeKey(z, x, y);
    if (memCache.contains(key))
        return memCache.value(key);

    QSqlQuery q(db);
    q.prepare("SELECT data FROM tiles WHERE z=? AND x=? AND y=?");
    q.addBindValue(z);
    q.addBindValue(x);
    q.addBindValue(y);
    if (!q.exec() || !q.next()) {
        return QImage(); // null image -> miss
    }
    QByteArray blob = q.value(0).toByteArray();
    QImage img;
    if (!img.loadFromData(blob)) {
        qWarning() << "TileCache: failed to decode image from DB for ("<<z<<','<<x<<','<<y<<')';
        return QImage();
    }
    if (memCache.size() >= MEM_CACHE_LIMIT) {
        memCache.remove(memCache.keys().first());
    }
    memCache.insert(key, img);
    return img;
}

/* -----------------------------------------------------------------
   Public API – putImage
   ----------------------------------------------------------------- */
void TileCache::putImage(int z, int x, int y, const QImage& img)
{
    if (img.isNull())
        return;

    QMutexLocker locker(&mtx);
    auto key = makeKey(z, x, y);
    if (memCache.size() >= MEM_CACHE_LIMIT) {
        memCache.remove(memCache.keys().first());
    }
    memCache.insert(key, img);

    QSqlQuery q(db);
    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    buf.close();

    q.prepare("INSERT OR REPLACE INTO tiles (z, x, y, data) VALUES (?, ?, ?, ?)");
    q.addBindValue(z);
    q.addBindValue(x);
    q.addBindValue(y);
    q.addBindValue(ba);
    if (!q.exec()) {
        qWarning() << "TileCache: DB insert failed:" << q.lastError().text();
    } else {
        emit imageReady(z, x, y);
    }
}

/* -----------------------------------------------------------------
   Optional pruning – simple size based LRU (timestamp column would be better)
   ----------------------------------------------------------------- */
void TileCache::pruneToSize(qint64 maxSizeBytes)
{
    QSqlQuery q(db);
    q.exec("SELECT SUM(LENGTH(data)) FROM tiles;");
    if (q.next()) {
        qint64 total = q.value(0).toLongLong();
        if (total > maxSizeBytes) {
            q.exec("DELETE FROM tiles WHERE rowid IN ("
                   "SELECT rowid FROM tiles ORDER BY rowid LIMIT "
                   "(SELECT COUNT(*)/2 FROM tiles));");
        }
    }
}