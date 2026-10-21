/* ProbeRunner.cpp
 *
 * Runs the on-device compatibility probes inside the application sandbox
 * and writes a report to:
 *
 *   /accounts/1000/appdata/com.compas.probes/probe-results.txt
 */

#include "ProbeRunner.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/qglobal.h>

/* ------------------------------------------------------------------ */
/* Probe registry                                                     */
/* ------------------------------------------------------------------ */

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
/* ProbeRunner                                                        */
/* ------------------------------------------------------------------ */

ProbeRunner::ProbeRunner(QObject* parent)
    : QObject(parent), m_results(QLatin1String("Running probes..."))
{
}

ProbeRunner::~ProbeRunner()
{
}

QString ProbeRunner::results() const
{
    return m_results;
}

QString ProbeRunner::resultsPath() const
{
    return resultsFilePath();
}

void ProbeRunner::rerun()
{
    m_results = runAll();
    emit resultsChanged();
}

QString ProbeRunner::runAll() const
{
    QString report;
    QTextStream out(&report);

    int passed = 0;
    int failed = 0;

    out << QString(QLatin1String("=== Compas BB10 On-Device Probe Suite ===\n"));
    out << QString(QLatin1String("started : %1\n"))
           .arg(QDateTime::currentDateTime().toString(Qt::ISODate));
    out << QString(QLatin1String("app id  : com.compas.probes\n"));
    out << QString(QLatin1String("target  : %1\n")).arg(QLatin1String(QT_ARCH));
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

void ProbeRunner::runProbe(const Probe& probe, QTextStream& out, int& passed, int& failed)
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
