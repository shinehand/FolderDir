#include "FileOperationDialog.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTimer>
#include <QtCore/QElapsedTimer>

// ──────────────────────────────────────────────────────────────────────────────
// ConflictDialog
// ──────────────────────────────────────────────────────────────────────────────
ConflictDialog::ConflictDialog(const QString &src, const QString &dst,
                               QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("File Conflict"));
    auto *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(
        tr("A file named <b>%1</b> already exists in the destination.")
            .arg(QFileInfo(dst).fileName()), this));
    layout->addWidget(new QLabel(
        tr("Source: %1\nDestination: %2").arg(src, dst), this));

    auto *btnLayout = new QHBoxLayout;
    auto makeBtn = [&](const QString &text, ConflictResolution res) {
        auto *btn = new QPushButton(text, this);
        connect(btn, &QPushButton::clicked, this, [this, res]() {
            m_resolution = res;
            accept();
        });
        btnLayout->addWidget(btn);
    };

    makeBtn(tr("Overwrite"),     ConflictResolution::Overwrite);
    makeBtn(tr("Overwrite All"), ConflictResolution::OverwriteAll);
    makeBtn(tr("Skip"),          ConflictResolution::Skip);
    makeBtn(tr("Skip All"),      ConflictResolution::SkipAll);
    makeBtn(tr("Rename"),        ConflictResolution::RenameNew);
    makeBtn(tr("Cancel"),        ConflictResolution::Cancel);

    layout->addLayout(btnLayout);
}

// ──────────────────────────────────────────────────────────────────────────────
// FileOperationDialog
// ──────────────────────────────────────────────────────────────────────────────
FileOperationDialog::FileOperationDialog(FileOperation *op, QWidget *parent)
    : QDialog(parent)
    , m_op(op)
{
    setupUi();

    connect(op, &FileOperation::progressChanged,
            this, &FileOperationDialog::onProgressChanged,
            Qt::QueuedConnection);
    connect(op, &FileOperation::conflictDetected,
            this, &FileOperationDialog::onConflictDetected,
            Qt::QueuedConnection);
    connect(op, &FileOperation::operationFinished,
            this, &FileOperationDialog::onOperationFinished,
            Qt::QueuedConnection);

    // Speed update timer
    m_elapsedTimer = new QElapsedTimer;
    m_elapsedTimer->start();
    m_timer = new QTimer(this);
    m_timer->setInterval(500);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_elapsed = m_elapsedTimer->elapsed();
    });
    m_timer->start();
}

void FileOperationDialog::setupUi()
{
    setWindowTitle(operationTitle(m_op->kind()));
    setMinimumWidth(440);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto *layout = new QVBoxLayout(this);

    m_titleLabel = new QLabel(operationTitle(m_op->kind()) + QStringLiteral("…"), this);
    QFont f = m_titleLabel->font();
    f.setBold(true);
    m_titleLabel->setFont(f);
    layout->addWidget(m_titleLabel);

    m_fileLabel = new QLabel(this);
    m_fileLabel->setWordWrap(true);
    layout->addWidget(m_fileLabel);

    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 100);
    layout->addWidget(m_progress);

    m_speedLabel = new QLabel(this);
    layout->addWidget(m_speedLabel);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &FileOperationDialog::onCancel);
    btnLayout->addWidget(m_cancelButton);
    layout->addLayout(btnLayout);
}

// ──────────────────────────────────────────────────────────────────────────────
void FileOperationDialog::onProgressChanged(int percent,
                                            const QString &currentFile,
                                            qint64 bytesDone,
                                            qint64 bytesTotal)
{
    m_progress->setValue(percent);
    m_fileLabel->setText(currentFile);

    // Speed calculation
    const qint64 elapsed = m_elapsedTimer->elapsed();
    if (elapsed > 0) {
        const double speed = bytesDone * 1000.0 / elapsed; // bytes/sec
        const qint64 remaining = bytesTotal - bytesDone;
        const double eta = speed > 0 ? remaining / speed : 0;
        m_speedLabel->setText(
            tr("%1/s — %2 remaining")
                .arg(formatBytes(static_cast<qint64>(speed)))
                .arg(tr("%n s", "", static_cast<int>(eta))));
    }
    m_lastBytes = bytesDone;
}

void FileOperationDialog::onConflictDetected(const QString &src,
                                             const QString &dst)
{
    ConflictDialog dlg(src, dst, this);
    dlg.exec();
    m_op->resolveConflict(dlg.resolution());
}

void FileOperationDialog::onOperationFinished(bool success,
                                              const QString &errorMsg)
{
    m_timer->stop();
    if (!success && !errorMsg.isEmpty()) {
        QMessageBox::warning(this, tr("Operation Failed"), errorMsg);
    }
    accept();
}

void FileOperationDialog::onCancel()
{
    m_op->cancel();
    reject();
}

// ──────────────────────────────────────────────────────────────────────────────
QString FileOperationDialog::formatBytes(qint64 bytes)
{
    return QLocale::system().formattedDataSize(
        bytes, 1, QLocale::DataSizeTraditionalFormat);
}

QString FileOperationDialog::operationTitle(FileOperationKind kind)
{
    switch (kind) {
    case FileOperationKind::Copy:   return tr("Copying");
    case FileOperationKind::Move:   return tr("Moving");
    case FileOperationKind::Delete: return tr("Deleting");
    }
    return tr("Working");
}
