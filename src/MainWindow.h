#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTabWidget>
#include <QtGui/QActionGroup>
#include <QtGui/QAction>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMenu>
#include <QtCore/QSettings>

#include "FileSystemBrowser.h"   // for ViewMode enum
#include "WorkspaceWidget.h"

class FolderPane;
class BookmarkManager;
class SettingsManager;
class DriveBar;
class SearchDialog;
class PreviewPanel;
class FolderTreePanel;
class LayoutManager;

/**
 * @brief MainWindow — top-level application window.
 *
 * Hosts a QTabWidget of WorkspaceWidget instances (workspace tabs).  Each
 * workspace independently manages 1–4 FolderPane instances in a 2×2 splitter
 * grid.  Every FolderPane carries its own BreadcrumbBar (address bar) and its
 * own inner tab bar for multiple directories.
 *
 * The top toolbar and menu bar always operate on the *active* workspace's
 * active pane.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // File menu
    void onNewFile();
    void onNewFolder();
    void onExit();

    // Edit menu
    void onCopy();
    void onCut();
    void onPaste();
    void onDelete();
    void onRename();
    void onSelectAll();
    void onCopyPath();

    // F-key operations
    void onCopyTo();        ///< F5 — copy selected to a destination path
    void onMoveTo();        ///< F6 — move selected to a destination path
    void onViewFile();      ///< F3 — open with viewer
    void onEditFile();      ///< F4 — open with editor
    void onProperties();    ///< Alt+Enter — show properties dialog

    // View menu
    void onLayout1Pane();
    void onLayout2PanesH();
    void onLayout2PanesV();
    void onLayout3Panes();
    void onLayout4Panes();
    void onToggleHidden();
    void onTogglePreview();
    void onToggleBookmarkSidebar();
    void onToggleFolderTree();
    void onSetViewMode(ViewMode mode); ///< Apply view mode to the active pane

    // SP-9: Panel sync / lock / clone (per active workspace)
    void onTogglePaneSync();    ///< Toggle directory-sync in the active workspace
    void onLockPane();          ///< Lock / unlock the active pane
    void onClonePane();         ///< Clone active pane's path to other panes

    // SP-10: Layout presets
    void onSaveLayoutPreset();
    void onLoadLayoutPreset(const QString &name);
    void onDeleteLayoutPreset(const QString &name);
    void rebuildLayoutPresetsMenu();

    // Workspace tabs
    void onAddWorkspace();
    void onCloseWorkspace(int index);
    void onWorkspaceTabChanged(int index);
    void onWorkspaceTabDoubleClicked(int index); ///< Rename workspace tab

    // Tools menu
    void onOpenSearch();
    void onOpenSettings();
    void onOpenTerminal();
    void onOpenColorRules();

    // Bookmarks menu
    void onAddBookmark();
    void onExportBookmarks();
    void onImportBookmarks();

    // Navigation
    void onNavigateBack();
    void onNavigateForward();
    void onNavigateUp();

    // Status / active pane
    void onActivePaneChanged(FolderPane *pane);
    void onSelectionChanged();
    void updateStatusBar();

    // Tree panel
    void onTreeNavigate(const QString &path);

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupDriveBar();
    void setupStatusBar();
    void setupDockWidgets();
    void saveSession();
    void restoreSession();
    void applyLayout(int paneCount);
    void syncPaneCountButtons();

    /** Connect a workspace's forwarded signals to MainWindow slots. */
    void connectWorkspace(WorkspaceWidget *ws);

    WorkspaceWidget *activeWorkspace() const;
    FolderPane      *activePane()      const;
    /** Path of the first other visible pane in the active workspace (F5/F6). */
    QString otherPanePath() const;

    // ── Workspace tab widget ─────────────────────────────────────────────────
    QTabWidget *m_workspaceTabs{nullptr};

    // ── Active pane (convenience pointer — always == activeWorkspace()->activePane()) ──
    FolderPane *m_activePane{nullptr};

    // ── Dock widgets ─────────────────────────────────────────────────────────
    QDockWidget *m_bookmarkDock{nullptr};
    QDockWidget *m_previewDock{nullptr};
    QDockWidget *m_treeDock{nullptr};

    PreviewPanel    *m_previewPanel{nullptr};
    FolderTreePanel *m_treePanel{nullptr};

    // ── Toolbar ──────────────────────────────────────────────────────────────
    QToolBar *m_toolBar{nullptr};
    DriveBar *m_driveBar{nullptr};

    // ── Status bar labels ────────────────────────────────────────────────────
    QLabel *m_statusSelection{nullptr};
    QLabel *m_statusItems{nullptr};
    QLabel *m_statusDisk{nullptr};

    // ── Actions ──────────────────────────────────────────────────────────────
    QAction *m_actHidden{nullptr};
    QAction *m_actPreview{nullptr};
    QAction *m_actBookmarkSidebar{nullptr};
    QAction *m_actFolderTree{nullptr};

    // SP-9: pane panel actions
    QAction *m_actPaneSync{nullptr};
    QAction *m_actLockPane{nullptr};
    QAction *m_actClonePane{nullptr};

    // View mode actions
    QAction *m_actViewDetails{nullptr};
    QAction *m_actViewList{nullptr};
    QAction *m_actViewIcons{nullptr};
    QAction *m_actViewThumbnails{nullptr};
    ViewMode m_currentViewMode{ViewMode::Details};

    // Pane-count toolbar actions
    QAction *m_actPane1{nullptr};
    QAction *m_actPane2{nullptr};
    QAction *m_actPane3{nullptr};
    QAction *m_actPane4{nullptr};

    // ── Bookmark list ────────────────────────────────────────────────────────
    QListWidget *m_bookmarkList{nullptr};

    // ── Managers ─────────────────────────────────────────────────────────────
    BookmarkManager *m_bookmarkManager{nullptr};
    SettingsManager *m_settingsManager{nullptr};
    LayoutManager   *m_layoutManager{nullptr};

    // SP-10: layout presets menu
    QMenu *m_layoutPresetsMenu{nullptr};
};

