#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <QtSql/QSqlDatabase>
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>
#include "TileCache.h"
#include "TileNetwork.h"

using namespace bb::cascades;

Q_DECL_EXPORT int main(int argc, char **argv)
{
    Application app(argc, argv);
    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(&app);

    // ----- Create TileCache (persisted in the app's data directory) -----
    // Qt 4.8.6 doesn't have QStandardPaths or QStringLiteral; use QDir + known BB10 path
    QString dataDir = QDir::homePath() + QLatin1String("/appdata/") +
                      QCoreApplication::applicationName();
    QDir().mkpath(dataDir);
    QString dbPath = dataDir + QLatin1String("/tilecache.db");

    // Alternative: use the known BB10 path
    QString dbPathFinal = QLatin1String("/accounts/1000/appdata/") +
                          QCoreApplication::applicationName() +
                          QLatin1String("/tilecache.db");
    QDir().mkpath(QLatin1String("/accounts/1000/appdata/") +
                  QCoreApplication::applicationName());

    TileCache* cache = new TileCache(dbPathFinal, qml);

    // ----- Create Network manager that talks to the cache -----
    TileNetwork* net = new TileNetwork(cache, qml);

    // Expose them to QML as context properties
    qml->setContextProperty("_tileCache", cache);
    qml->setContextProperty("_networkMgr", net);

    AbstractPane *root = qml->createRootObject<AbstractPane>();
    app.setScene(root);
    return Application::exec();
}