#include "FileSystemModel.h"
#include "FolderSizeWorker.h"
#include "ColorManager.h"

#include <QtCore/QDir>
#include <QtCore/QLocale>
#include <QtGui/QBrush>

// ──────────────────────────────────────────────────────────────────────────────
FileSystemModel::FileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{
    setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);

    m_sizeWorker = new FolderSizeWorker(this);
    connect(m_sizeWorker, &FolderSizeWorker::sizeReady,
            this, &FileSystemModel::onFolderSizeReady);

    // Refresh view when colour rules change
    connect(ColorManager::instance(), &ColorManager::rulesChanged,
            this, [this]() {
        // Notify the entire model so all visible items repaint their colours
        emit dataChanged(QModelIndex{}, QModelIndex{},
                         {Qt::BackgroundRole, Qt::ForegroundRole});
    });
}

// ──────────────────────────────────────────────────────────────────────────────
void FileSystemModel::setNameFilterPattern(const QString &pattern)
{
    m_pattern = pattern;
    if (pattern.isEmpty()) {
        setNameFilters(QStringList());
        setNameFilterDisables(false);
    } else {
        // Split comma-separated patterns: "*.cpp, *.h" → ["*.cpp","*.h"]
        QStringList filters;
        const QStringList parts = pattern.split(QLatin1Char(','));
        for (const QString &p : parts) {
            const QString trimmed = p.trimmed();
            if (!trimmed.isEmpty()) filters << trimmed;
        }
        setNameFilters(filters);
        setNameFilterDisables(false);
    }
}

// ──────────────────────────────────────────────────────────────────────────────
QVariant FileSystemModel::headerData(int section,
                                     Qt::Orientation orientation,
                                     int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return tr("Name");
        case 1: return tr("Size");
        case 2: return tr("Type");
        case 3: return tr("Date Modified");
        default: break;
        }
    }
    return QFileSystemModel::headerData(section, orientation, role);
}

// ──────────────────────────────────────────────────────────────────────────────
QVariant FileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QFileSystemModel::data(index, role);

    // Human-readable size in the Size column (column 1)
    if (index.column() == 1 && role == Qt::DisplayRole) {
        const QFileInfo fi = fileInfo(index);
        if (fi.isDir()) {
            const QString absPath = fi.absoluteFilePath();
            if (m_folderSizes.contains(absPath)) {
                return QLocale::system().formattedDataSize(
                    m_folderSizes.value(absPath), 2,
                    QLocale::DataSizeTraditionalFormat);
            }
            // Kick off async computation and show placeholder
            m_sizeWorker->requestSize(absPath);
            return tr("…");
        }
        const qint64 bytes = fi.size();
        return QLocale::system().formattedDataSize(
            bytes, 2, QLocale::DataSizeTraditionalFormat);
    }

    // Colour coding
    if (role == Qt::BackgroundRole || role == Qt::ForegroundRole) {
        const QString name = fileInfo(index).fileName();
        QColor bg, fg;
        if (ColorManager::instance()->colorsForFile(name, bg, fg)) {
            if (role == Qt::BackgroundRole && bg.isValid())
                return QBrush(bg);
            if (role == Qt::ForegroundRole && fg.isValid())
                return QBrush(fg);
        }
        return QVariant();
    }

    return QFileSystemModel::data(index, role);
}

// ──────────────────────────────────────────────────────────────────────────────
void FileSystemModel::onFolderSizeReady(const QString &path, qint64 bytes)
{
    m_folderSizes.insert(path, bytes);
    const QModelIndex idx = index(path, 1);
    if (idx.isValid())
        emit dataChanged(idx, idx, {Qt::DisplayRole});
}
