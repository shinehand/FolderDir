#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QListView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtCore/QStringList>

class FileSystemModel;

/**
 * @brief ViewMode — how files are displayed in a FileSystemBrowser.
 */
enum class ViewMode {
    Details,    ///< Table with name/size/type/date columns (QTreeView)
    List,       ///< Multi-column icon list (QListView)
    Icons,      ///< Large icons (QListView + IconMode)
    Thumbnails  ///< Image thumbnails (QListView + IconMode + thumb delegate)
};

/**
 * @brief FileSystemBrowser — displays the contents of a single directory.
 *
 * Uses a QTreeView (Details mode) or QListView (other modes) backed by a
 * custom FileSystemModel.  Supports:
 *  - Sorting by column
 *  - In-place rename (F2)
 *  - Context menu with file operations
 *  - Keyboard navigation
 *  - Filter bar (wildcard/glob, shown at the bottom)
 *  - Drag-and-drop source and target
 */
class FileSystemBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit FileSystemBrowser(const QString &path = QString(),
                               QWidget *parent = nullptr);
    ~FileSystemBrowser() override;

    QString currentPath() const;
    ViewMode viewMode() const { return m_viewMode; }
    void setViewMode(ViewMode mode);
    bool showHidden() const;
    void setShowHidden(bool show);

    QStringList selectedPaths() const;
    QAbstractItemView *view() const;

public slots:
    void setPath(const QString &path);
    void refresh();
    void navigateUp();
    void navigateBack();
    void navigateForward();
    void beginRename();
    void copySelected();
    void cutSelected();
    void pasteHere();
    void deleteSelected(bool permanent = false);
    void newFolder();
    void newFile();
    void showProperties();
    void copyPathToClipboard();
    void toggleFilterBar();
    void setFilter(const QString &pattern);
    void selectAll();

signals:
    void pathChanged(const QString &newPath);
    void selectionChanged();
    void fileActivated(const QString &path);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onItemActivated(const QModelIndex &index);
    void onContextMenu(const QPoint &pos);
    void onFilterChanged(const QString &text);
    void onSortIndicatorChanged(int column, Qt::SortOrder order);

private:
    void setupUi();
    void setupDetailsView();
    void setupListView();
    void switchToView(QAbstractItemView *view);
    void applySort();
    QString pathForIndex(const QModelIndex &index) const;
    bool confirmDelete(int count) const;

    QString           m_currentPath;
    ViewMode          m_viewMode{ViewMode::Details};

    FileSystemModel  *m_model{nullptr};

    QTreeView        *m_detailsView{nullptr};
    QListView        *m_listView{nullptr};
    QAbstractItemView *m_activeView{nullptr};

    QVBoxLayout      *m_layout{nullptr};
    QHBoxLayout      *m_filterLayout{nullptr};
    QWidget          *m_filterBar{nullptr};
    QLineEdit        *m_filterEdit{nullptr};
    QLabel           *m_filterLabel{nullptr};

    QStringList       m_backHistory;
    QStringList       m_forwardHistory;
    QString           m_clipboard;   ///< path list for internal copy/cut
    bool              m_cutMode{false};
};
