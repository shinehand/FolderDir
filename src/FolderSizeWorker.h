#pragma once

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QMutex>

/**
 * @brief FolderSizeWorker — computes folder sizes asynchronously.
 *
 * Call requestSize() for any directory path.  When the size is ready the
 * sizeReady() signal is emitted from the worker thread, so connect with
 * Qt::QueuedConnection (or just emit to the main thread via invokeMethod).
 */
class FolderSizeWorker : public QObject
{
    Q_OBJECT
public:
    explicit FolderSizeWorker(QObject *parent = nullptr);

    /** Queue a folder-size computation.  No-op if already pending/done. */
    void requestSize(const QString &path);

signals:
    void sizeReady(const QString &path, qint64 bytes);

private:
    QSet<QString> m_pending;
    QMutex        m_mutex;

    static qint64 computeSize(const QString &path);
};
