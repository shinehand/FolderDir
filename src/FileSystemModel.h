#pragma once

#include <QtGui/QFileSystemModel>
#include <QtCore/QSortFilterProxyModel>

/**
 * @brief FileSystemModel — thin wrapper around QFileSystemModel.
 *
 * Adds:
 *  - Glob/wildcard filter on file names (not directory names)
 *  - Human-readable file sizes
 *  - Consistent column ordering: Name | Size | Type | Date Modified
 */
class FileSystemModel : public QFileSystemModel
{
    Q_OBJECT

public:
    explicit FileSystemModel(QObject *parent = nullptr);

    /** Set a glob pattern, e.g. "*.cpp" or "img_*". Empty = show all. */
    void setNameFilterPattern(const QString &pattern);
    QString nameFilterPattern() const { return m_pattern; }

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

private:
    QString m_pattern;
};
