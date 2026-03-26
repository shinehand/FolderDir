#include "FolderSizeWorker.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFutureWatcher>
#include <QtCore/QDirIterator>
#include <QtCore/QMutexLocker>

// ──────────────────────────────────────────────────────────────────────────────
FolderSizeWorker::FolderSizeWorker(QObject *parent)
    : QObject(parent)
{}

// ──────────────────────────────────────────────────────────────────────────────
void FolderSizeWorker::requestSize(const QString &path)
{
    {
        QMutexLocker lock(&m_mutex);
        if (m_pending.contains(path)) return;
        m_pending.insert(path);
    }

    auto *watcher = new QFutureWatcher<qint64>(this);
    connect(watcher, &QFutureWatcher<qint64>::finished,
            this, [this, watcher, path]() {
        const qint64 size = watcher->result();
        {
            QMutexLocker lock(&m_mutex);
            m_pending.remove(path);
        }
        emit sizeReady(path, size);
        watcher->deleteLater();
    });
    watcher->setFuture(QtConcurrent::run(&FolderSizeWorker::computeSize, path));
}

// ──────────────────────────────────────────────────────────────────────────────
qint64 FolderSizeWorker::computeSize(const QString &path)
{
    qint64 total = 0;
    QDirIterator it(path,
                    QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        if (it.fileInfo().isFile())
            total += it.fileInfo().size();
    }
    return total;
}
