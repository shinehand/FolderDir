#pragma once

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QAtomicInt>
#include <functional>

/**
 * @brief ConflictResolution — user choice when a destination file exists.
 */
enum class ConflictResolution {
    Overwrite,
    Skip,
    RenameNew,
    OverwriteAll,
    SkipAll,
    Cancel
};

/**
 * @brief FileOperationKind — type of a file operation.
 */
enum class FileOperationKind {
    Copy,
    Move,
    Delete
};

/**
 * @brief FileOperation — encapsulates an async file operation.
 *
 * Runs in a dedicated QThread.  Progress and conflict callbacks are
 * marshalled back to the GUI thread via Qt signals.
 */
class FileOperation : public QObject
{
    Q_OBJECT

public:
    explicit FileOperation(FileOperationKind kind,
                           const QStringList &sources,
                           const QString &destination,
                           QObject *parent = nullptr);

    void start();
    void cancel();
    bool isCancelled() const;

    FileOperationKind kind() const { return m_kind; }
    QStringList sources() const { return m_sources; }
    QString destination() const { return m_destination; }

    /** Called from GUI thread to resolve a conflict; unblocks the worker. */
    void resolveConflict(ConflictResolution resolution);

signals:
    void progressChanged(int percent, const QString &currentFile,
                         qint64 bytesDone, qint64 bytesTotal);
    void conflictDetected(const QString &src, const QString &dst);
    void operationFinished(bool success, const QString &errorMessage);
    void fileCompleted(const QString &path);

private slots:
    void run();

private:
    bool copyFile(const QString &src, const QString &dst);
    bool copyDirectory(const QString &src, const QString &dst);
    bool deleteRecursive(const QString &path);
    QString resolveDestPath(const QString &src) const;
    qint64 totalSize(const QStringList &paths) const;

    FileOperationKind m_kind;
    QStringList       m_sources;
    QString           m_destination;

    QAtomicInt         m_cancelled{0};
    QMutex             m_conflictMutex;
    QWaitCondition     m_conflictWait;
    ConflictResolution m_conflictResolution{ConflictResolution::Skip};
    bool               m_waitingForConflict{false};

    QThread           *m_thread{nullptr};
};
