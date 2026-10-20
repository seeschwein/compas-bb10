#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <QtSql/QSqlDatabase>
#include <QStandardPaths>
#include "TileCache.h"
#include "TileNetwork.h"

using namespace bb::cascades;

Q_DECL_EXPORT int main(int argc, char **argv)
{
    Application app(argc, argv);
    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(&app);

    // ----- Create TileCache (persisted in the app's data directory) -----
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) +
                     QLatin1String("/tilecache.db");
    TileCache* cache = new TileCache(dbPath, qml);

    // ----- Create Network manager that talks to the cache -----
    TileNetwork* net = new TileNetwork(cache, qml);

    // Expose them to QML as context properties
    qml->setContextProperty("_tileCache", cache);
    qml->setContextProperty("_networkMgr", net);

    AbstractPane *root = qml->createRootObject<AbstractPane>();
    app.setScene(root);
    return Application::exec();
}