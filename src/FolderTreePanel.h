#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>

class QFileSystemModel;

/**
 * @brief FolderTreePanel — directory-hierarchy sidebar.
 *
 * Displays:
 *  1. A "Quick Access" pinned list at the top (Desktop, Documents, Downloads,
 *     Home) populated via QStandardPaths — UX-B06.
 *  2. A QTreeView showing the full file-system hierarchy (directories only).
 *
 * Clicking (single-click) any item emits navigateRequested(path) so the
 * active FolderPane can navigate there.
 *
 * Call setActivePath() whenever the active pane's directory changes so
 * that the tree automatically expands and selects the matching node.
 */
class FolderTreePanel : public QWidget
{
    Q_OBJECT

public:
    explicit FolderTreePanel(QWidget *parent = nullptr);

    /**
     * Expands and selects the tree node that corresponds to @p path.
     * Call this whenever the active pane navigates to a new directory.
     */
    void setActivePath(const QString &path);

signals:
    /** Emitted when the user clicks a folder in the tree or quick-access list. */
    void navigateRequested(const QString &path);

private slots:
    void onClicked(const QModelIndex &index);
    void onQuickAccessClicked(QListWidgetItem *item);

private:
    void setupUi();
    void buildQuickAccess();

    QFileSystemModel *m_model{nullptr};
    QTreeView        *m_treeView{nullptr};
    QListWidget      *m_quickAccess{nullptr};
    QVBoxLayout      *m_layout{nullptr};

    bool              m_syncInProgress{false}; ///< Guards against re-entrant expansion
};
