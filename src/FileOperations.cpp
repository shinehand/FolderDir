#include "FileOperations.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDirIterator>
#include <QtCore/QThread>
#include <QtCore/QMutexLocker>
#include <QtCore/QWaitCondition>

// ──────────────────────────────────────────────────────────────────────────────
FileOperation::FileOperation(FileOperationKind kind,
                             const QStringList &sources,
                             const QString &destination,
                             QObject *parent)
    : QObject(parent)
    , m_kind(kind)
    , m_sources(sources)
    , m_destination(destination)
{
}

// ──────────────────────────────────────────────────────────────────────────────
void FileOperation::start()
{
    // Note: m_thread has no parent so it outlives the moveToThread call.
    // FileOperationDialog owns the FileOperation and takes care of cleanup
    // after the operationFinished signal is received.
    m_thread = new QThread;
    connect(m_thread, &QThread::started,  this,     &FileOperation::run);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    // Signals emitted by the worker thread will be delivered to GUI via
    // queued connections because the slots (in FileOperationDialog) live in
    // the GUI thread.
    moveToThread(m_thread);
    m_thread->start();
}

void FileOperation::cancel()
{
    m_cancelled.storeRelaxed(1);
    // Wake any thread blocked waiting for conflict resolution
    QMutexLocker lk(&m_conflictMutex);
    m_waitingForConflict = false;
    m_conflictWait.wakeAll();
}

bool FileOperation::isCancelled() const
{
    return m_cancelled.loadRelaxed() != 0;
}

void FileOperation::resolveConflict(ConflictResolution resolution)
{
    QMutexLocker lk(&m_conflictMutex);
    m_conflictResolution = resolution;
    m_waitingForConflict = false;
    m_conflictWait.wakeAll();
}

// ──────────────────────────────────────────────────────────────────────────────
void FileOperation::run()
{
    const qint64 total = totalSize(m_sources);
    qint64 done = 0;
    QString errorMsg;
    bool success = true;

    for (const QString &src : m_sources) {
        if (isCancelled()) { success = false; break; }

        const QFileInfo fi(src);
        if (!fi.exists()) continue;

        if (m_kind == FileOperationKind::Delete) {
            if (!deleteRecursive(src)) {
                errorMsg = tr("Failed to delete: %1").arg(src);
                success = false;
            }
        } else {
            const QString dst = resolveDestPath(src);
            if (fi.isDir()) {
                if (!copyDirectory(src, dst)) {
                    errorMsg = tr("Failed to copy directory: %1").arg(src);
                    success = false;
                }
            } else {
                // Check conflict
                if (QFile::exists(dst)) {
                    QMutexLocker lk(&m_conflictMutex);
                    m_waitingForConflict = true;
                    m_conflictResolution = ConflictResolution::Skip;
                    emit conflictDetected(src, dst);
                    // Block the worker thread using QWaitCondition until
                    // the GUI thread calls resolveConflict().
                    while (m_waitingForConflict && !isCancelled()) {
                        m_conflictWait.wait(&m_conflictMutex, 100 /*ms timeout*/);
                    }
                    if (isCancelled()) { success = false; break; }

                    const ConflictResolution res = m_conflictResolution;
                    lk.unlock();

                    if (res == ConflictResolution::Cancel) {
                        success = false; break;
                    }
                    if (res == ConflictResolution::Skip ||
                        res == ConflictResolution::SkipAll) {
                        emit fileCompleted(src);
                        continue;
                    }
                    if (res == ConflictResolution::Overwrite ||
                        res == ConflictResolution::OverwriteAll) {
                        QFile::remove(dst);
                    }
                    // RenameNew: copyFile will create a unique name automatically
                }

                if (!copyFile(src, dst)) {
                    errorMsg = tr("Failed to copy: %1").arg(src);
                    success = false;
                } else if (m_kind == FileOperationKind::Move) {
                    QFile::remove(src);
                }
                done += fi.size();
                emit progressChanged(
                    total > 0 ? static_cast<int>(done * 100 / total) : 50,
                    fi.fileName(), done, total);
            }
        }
        emit fileCompleted(src);
    }

    emit operationFinished(success, errorMsg);
}

// ──────────────────────────────────────────────────────────────────────────────
bool FileOperation::copyFile(const QString &src, const QString &dst)
{
    // Ensure destination directory exists
    QDir().mkpath(QFileInfo(dst).absolutePath());
    return QFile::copy(src, dst);
}

bool FileOperation::copyDirectory(const QString &src, const QString &dst)
{
    QDir srcDir(src);
    if (!QDir().mkpath(dst)) return false;

    const QFileInfoList entries =
        srcDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (const QFileInfo &fi : entries) {
        if (isCancelled()) return false;
        const QString dstPath = dst + QDir::separator() + fi.fileName();
        if (fi.isDir()) {
            if (!copyDirectory(fi.absoluteFilePath(), dstPath)) return false;
        } else {
            if (!copyFile(fi.absoluteFilePath(), dstPath)) return false;
            if (m_kind == FileOperationKind::Move)
                QFile::remove(fi.absoluteFilePath());
        }
    }
    if (m_kind == FileOperationKind::Move)
        srcDir.rmdir(src);
    return true;
}

bool FileOperation::deleteRecursive(const QString &path)
{
    QFileInfo fi(path);
    if (fi.isDir()) {
        QDir dir(path);
        const QFileInfoList entries =
            dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        for (const QFileInfo &child : entries) {
            if (!deleteRecursive(child.absoluteFilePath())) return false;
        }
        return dir.rmdir(path);
    }
    return QFile::remove(path);
}

QString FileOperation::resolveDestPath(const QString &src) const
{
    const QString fileName = QFileInfo(src).fileName();
    return m_destination + QDir::separator() + fileName;
}

qint64 FileOperation::totalSize(const QStringList &paths) const
{
    qint64 total = 0;
    for (const QString &p : paths) {
        QFileInfo fi(p);
        if (fi.isFile()) {
            total += fi.size();
        } else if (fi.isDir()) {
            QDirIterator it(p, QDir::Files | QDir::NoDotAndDotDot,
                            QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                total += it.fileInfo().size();
            }
        }
    }
    return total;
}
