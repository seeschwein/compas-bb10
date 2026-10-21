/* ProbeRunner.h
 *
 * Runs the on-device compatibility probes and exposes the report to QML.
 */

#ifndef PROBERUNNER_H
#define PROBERUNNER_H

#include <QtCore/QObject>
#include <QtCore/QString>

class QTextStream;

/* One probe executable and what it exercises. */
struct Probe
{
    const char* name;
    const char* binary;   /* relative to the probe directory */
    const char* purpose;
};

class ProbeRunner : public QObject
{
    Q_OBJECT
public:
    explicit ProbeRunner(QObject* parent = 0);
    ~ProbeRunner();

    /* The full report text, shown in the UI. */
    Q_PROPERTY(QString results READ results NOTIFY resultsChanged)
    QString results() const;

    /* Re-run the whole suite; updates the property and rewrites the file. */
    Q_INVOKABLE void rerun();

    /* The on-disk location of the report. */
    Q_PROPERTY(QString resultsPath READ resultsPath CONSTANT)
    QString resultsPath() const;

signals:
    void resultsChanged();

private:
    QString m_results;
    QString runAll() const;
    static void runProbe(const Probe& probe, QTextStream& out, int& passed, int& failed);
};

#endif // PROBERUNNER_H
