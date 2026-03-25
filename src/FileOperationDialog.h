#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QDialogButtonBox>
#include "FileOperations.h"

/**
 * @brief ConflictDialog — shown when a copy/move encounters an existing file.
 */
class ConflictDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConflictDialog(const QString &src, const QString &dst,
                            QWidget *parent = nullptr);
    ConflictResolution resolution() const { return m_resolution; }

private:
    ConflictResolution m_resolution{ConflictResolution::Skip};
};

/**
 * @brief FileOperationDialog — progress dialog for async file operations.
 *
 * Connects to a FileOperation object, shows progress, handles conflict
 * resolution sub-dialogs, and allows cancellation.
 */
class FileOperationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileOperationDialog(FileOperation *op, QWidget *parent = nullptr);

private slots:
    void onProgressChanged(int percent, const QString &currentFile,
                           qint64 bytesDone, qint64 bytesTotal);
    void onConflictDetected(const QString &src, const QString &dst);
    void onOperationFinished(bool success, const QString &errorMsg);
    void onCancel();

private:
    void setupUi();
    static QString formatBytes(qint64 bytes);
    static QString operationTitle(FileOperationKind kind);

    FileOperation  *m_op{nullptr};

    QLabel         *m_titleLabel{nullptr};
    QLabel         *m_fileLabel{nullptr};
    QLabel         *m_speedLabel{nullptr};
    QProgressBar   *m_progress{nullptr};
    QPushButton    *m_cancelButton{nullptr};

    qint64          m_lastBytes{0};
    qint64          m_elapsed{0};  // ms since start
    class QTimer   *m_timer{nullptr};
    class QElapsedTimer *m_elapsedTimer{nullptr};
};
