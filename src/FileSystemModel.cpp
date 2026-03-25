#include "FileSystemModel.h"

#include <QtCore/QDir>
#include <QtCore/QLocale>

// ──────────────────────────────────────────────────────────────────────────────
FileSystemModel::FileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{
    // Show all file types by default; directories are always shown.
    setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
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
    // Human-readable size in the Size column (column 1)
    if (index.isValid() && index.column() == 1 && role == Qt::DisplayRole) {
        const QFileInfo fi = fileInfo(index);
        if (fi.isDir()) return QString(); // directories show no size
        const qint64 bytes = fi.size();
        return QLocale::system().formattedDataSize(
            bytes, 2, QLocale::DataSizeTraditionalFormat);
    }
    return QFileSystemModel::data(index, role);
}
