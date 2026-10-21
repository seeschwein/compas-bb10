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
 * The report is also exposed to QML as the context property "probeResults"
 * so the UI can display it without needing QML file I/O.
 */

#include <bb/cascades/AbstractPane>
#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QSysInfo>
#include <QtCore/QTextStream>

using namespace bb::cascades;

/* ------------------------------------------------------------------ */
/* Probe registry                                                     */
/* ------------------------------------------------------------------ */

struct Probe
{
    const char* name;
    const char* binary;   /* relative to the probe directory */
    const char* purpose;
};

static const Probe kProbes[] = {
    { "filesystem_probe", "filesystem_probe", "std::filesystem, paths, directory iteration" },
    { "posix_probe",      "posix_probe",      "stat, mmap, pipes, sockets, permissions" },
    { "thread_probe",     "thread_probe",     "threads, mutexes, condition variables, atomics" },
    { "loader_probe",     "loader_probe",     "dlopen, dlsym, dlclose, C ABI, plugin exceptions" }
};

static const int kProbeCount = sizeof(kProbes) / sizeof(kProbes[0]);

/* ------------------------------------------------------------------ */
/* Paths                                                              */
/* ------------------------------------------------------------------ */

/* The BAR unpacks into /app/native.  The probes live in a "probes"
 * subdirectory next to the runner binary. */
static QString probeDirPath()
{
    return QString(QLatin1String("/app/native/probes"));
}

static QString resultsDirPath()
{
    return QString(QLatin1String("/accounts/1000/appdata/com.compas.probes"));
}

static QString resultsFilePath()
{
    return resultsDirPath() + QLatin1String("/probe-results.txt");
}

/* ------------------------------------------------------------------ */
/* Runner object exposed to QML                                       */
/* ------------------------------------------------------------------ */

class ProbeRunner : public QObject
{
    Q_OBJECT
public:
    explicit ProbeRunner(QObject* parent = 0)
        : QObject(parent), m_results(QLatin1String("Running probes..."))
    {
    }

    /* The full report text, shown in the UI. */
    Q_PROPERTY(QString results READ results NOTIFY resultsChanged)
    QString results() const { return m_results; }

    /* Re-run the whole suite; updates the property and rewrites the file. */
    Q_INVOKABLE void rerun()
    {
        m_results = runAll();
        emit resultsChanged();
    }

    /* The on-disk location of the report. */
    Q_PROPERTY(QString resultsPath READ resultsPath CONSTANT)
    QString resultsPath() const { return resultsFilePath(); }

signals:
    void resultsChanged();

private:
    QString m_results;

    QString runAll() const
    {
        QString report;
        QTextStream out(&report);

        int passed = 0;
        int failed = 0;

        out << QString(QLatin1String("=== Compas BB10 On-Device Probe Suite ===\n"));
        out << QString(QLatin1String("started : %1\n"))
               .arg(QDateTime::currentDateTime().toString(Qt::ISODate));
        out << QString(QLatin1String("app id  : com.compas.probes\n"));
        out << QString(QLatin1String("target  : %1\n")).arg(QSysInfo::machine());
        out << QString(QLatin1String("probes  : %1\n\n")).arg(kProbeCount);

        for (int i = 0; i < kProbeCount; ++i)
            runProbe(kProbes[i], out, passed, failed);

        out << QString(QLatin1String("=== Summary ===\n"));
        out << QString(QLatin1String("passed: %1\n")).arg(passed);
        out << QString(QLatin1String("failed: %1\n")).arg(failed);
        out << QString(QLatin1String("finished: %1\n"))
               .arg(QDateTime::currentDateTime().toString(Qt::ISODate));

        out.flush();

        /* Persist to the app data directory so the results can be pulled
         * off the device later. */
        QDir().mkpath(resultsDirPath());
        QFile file(resultsFilePath());
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream f(&file);
            f << report;
            f.flush();
            file.close();
        } else {
            qWarning("probe_runner: cannot write %s", qPrintable(resultsFilePath()));
        }

        return report;
    }

    static void runProbe(const Probe& probe, QTextStream& out, int& passed, int& failed)
    {
        const QString binary = probeDirPath() + QLatin1String("/") + QLatin1String(probe.binary);

        out << QString(QLatin1String("--- %1 ---\n")).arg(QLatin1String(probe.name));
        out << QString(QLatin1String("purpose: %1\n")).arg(QLatin1String(probe.purpose));
        out << QString(QLatin1String("binary : %1\n")).arg(binary);

        if (!QFile::exists(binary)) {
            out << QString(QLatin1String("RESULT: FAIL (binary not found)\n\n"));
            ++failed;
            return;
        }

        QProcess process;
        process.setWorkingDirectory(probeDirPath());
        process.start(binary, QStringList());

        if (!process.waitForStarted(5000)) {
            out << QString(QLatin1String("RESULT: FAIL (could not start: %1)\n\n"))
                   .arg(process.errorString());
            ++failed;
            return;
        }

        if (!process.waitForFinished(30000)) {
            process.kill();
            process.waitForFinished(2000);
            out << QString(QLatin1String("RESULT: FAIL (timed out)\n\n"));
            ++failed;
            return;
        }

        const QString stdoutText = QString::fromLocal8Bit(process.readAllStandardOutput());
        const QString stderrText = QString::fromLocal8Bit(process.readAllStandardError());
        const int exitCode = process.exitCode();

        if (!stdoutText.isEmpty())
            out << stdoutText;
        if (!stderrText.isEmpty()) {
            out << QString(QLatin1String("[stderr]\n"));
            out << stderrText;
        }

        out << QString(QLatin1String("exit code: %1\n")).arg(exitCode);

        if (exitCode == 0) {
            out << QString(QLatin1String("RESULT: PASS\n\n"));
            ++passed;
        } else {
            out << QString(QLatin1String("RESULT: FAIL\n\n"));
            ++failed;
        }
    }
};

/* ------------------------------------------------------------------ */
/* Entry point                                                        */
/* ------------------------------------------------------------------ */

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
