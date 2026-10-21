#include <bb/cascades/AbstractPane>
#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>

using namespace bb::cascades;
Q_DECL_EXPORT int main(int argc, char **argv)
{
    Application app(argc, argv);
    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(&app);
    AbstractPane *root = qml->createRootObject<AbstractPane>();
    app.setScene(root);
    return Application::exec();
}
