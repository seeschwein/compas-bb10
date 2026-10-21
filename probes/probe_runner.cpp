/* probe_runner.cpp
 *
 * On-device probe runner for BlackBerry 10.
 *
 * This is the BAR entry point (an ELF binary, as required by the BB10
 * packager -- a shell script is NOT a valid entry point).  It runs each
 * probe executable inside the application sandbox, captures stdout/stderr
 * and the exit code, and writes a human readable report to the
 * application's writable data directory:
 *
 *   /accounts/1000/appdata/com.compas.probes/probe-results.txt
 *
 * The report is also exposed to QML as the context property "probeRunner"
 * so the UI can display it without needing QML file I/O.
 */

#include <bb/cascades/AbstractPane>
#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>

#include "ProbeRunner.h"

using namespace bb::cascades;

Q_DECL_EXPORT int main(int argc, char **argv)
{
    Application app(argc, argv);

    ProbeRunner* runner = new ProbeRunner(&app);

    QmlDocument* qml = QmlDocument::create("asset:///main.qml").parent(&app);
    qml->setContextProperty("probeRunner", runner);

    /* Run the suite before the scene is shown so the results are on disk
     * even if the user never touches the UI. */
    runner->rerun();

    AbstractPane* root = qml->createRootObject<AbstractPane>();
    app.setScene(root);

    return Application::exec();
}
