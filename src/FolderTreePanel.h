#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>

class QFileSystemModel;

/**
 * @brief FolderTreePanel — directory-hierarchy sidebar.
 *
 * Displays a QTreeView backed by QFileSystemModel (directories only).
 * Clicking (single-click) a directory emits navigateRequested(path) so
 * the active FolderPane can navigate there.
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
    /** Emitted when the user clicks a folder in the tree. */
    void navigateRequested(const QString &path);

private slots:
    void onClicked(const QModelIndex &index);

private:
    void setupUi();

    QFileSystemModel *m_model{nullptr};
    QTreeView        *m_treeView{nullptr};
    QVBoxLayout      *m_layout{nullptr};

    bool              m_syncInProgress{false}; ///< Guards against re-entrant expansion
};
