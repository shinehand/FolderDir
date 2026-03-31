#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSplitter>
#include <QtGui/QActionGroup>
#include <QtGui/QAction>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMenu>
#include <QtCore/QSettings>
#include <array>

#include "FileSystemBrowser.h"   // for ViewMode enum

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
 * Hosts up to four FolderPane instances arranged in a 2×2 grid of
 * QSplitters, plus a menu bar, toolbar, drive bar, status bar, and
 * optional side panels (bookmarks, preview, folder tree).
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
    void onSetViewMode(ViewMode mode); ///< Apply view mode to all visible panes

    // SP-9: Panel sync / lock / clone
    void onTogglePaneSync();    ///< Toggle directory-sync across all visible panes
    void onLockPane();          ///< Lock / unlock the active pane (prevent navigation)
    void onClonePane();         ///< Clone active pane's current path to other panes

    // SP-10: Layout presets
    void onSaveLayoutPreset();  ///< Prompt user for a name and save current layout
    void onLoadLayoutPreset(const QString &name); ///< Restore a saved layout preset
    void onDeleteLayoutPreset(const QString &name); ///< Remove a saved preset
    void rebuildLayoutPresetsMenu(); ///< Rebuild the dynamic presets sub-menu

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

    // Status
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
    void setupPanes(int count);
    void setupStatusBar();
    void setupDockWidgets();
    void setupConnections();
    void saveSession();
    void restoreSession();
    void applyLayout(int paneCount);

    FolderPane *activePane() const;
    /** Returns the path of the first other visible pane (for F5/F6 default dest). */
    QString otherPanePath() const;

    // Panes (max 4)
    std::array<FolderPane *, 4> m_panes{};
    int m_paneCount{4};
    FolderPane *m_activePane{nullptr};

    // Layout splitters
    QSplitter *m_hSplitter{nullptr};   // top / bottom
    QSplitter *m_topSplitter{nullptr}; // top-left / top-right
    QSplitter *m_botSplitter{nullptr}; // bot-left / bot-right

    // Dock widgets
    QDockWidget *m_bookmarkDock{nullptr};
    QDockWidget *m_previewDock{nullptr};
    QDockWidget *m_treeDock{nullptr};

    PreviewPanel    *m_previewPanel{nullptr};
    FolderTreePanel *m_treePanel{nullptr};

    // Toolbar
    QToolBar *m_toolBar{nullptr};
    DriveBar *m_driveBar{nullptr};

    // Status bar labels
    QLabel *m_statusSelection{nullptr};
    QLabel *m_statusItems{nullptr};
    QLabel *m_statusDisk{nullptr};

    // Actions
    QAction *m_actHidden{nullptr};
    QAction *m_actPreview{nullptr};
    QAction *m_actBookmarkSidebar{nullptr};
    QAction *m_actFolderTree{nullptr};

    // SP-9: pane panel actions
    QAction *m_actPaneSync{nullptr};   ///< Toggle sync-navigation
    QAction *m_actLockPane{nullptr};   ///< Lock/unlock active pane
    QAction *m_actClonePane{nullptr};  ///< Clone active pane path to others

    // SP-9: sync state
    bool m_paneSyncEnabled{false};

    // View mode actions (exclusive group in View > View Mode sub-menu)
    QAction *m_actViewDetails{nullptr};
    QAction *m_actViewList{nullptr};
    QAction *m_actViewIcons{nullptr};
    QAction *m_actViewThumbnails{nullptr};
    ViewMode m_currentViewMode{ViewMode::Details};

    // Pane-count toolbar actions (exclusive group; checked = active count)
    QAction *m_actPane1{nullptr};
    QAction *m_actPane2{nullptr};
    QAction *m_actPane3{nullptr};
    QAction *m_actPane4{nullptr};

    // Bookmark list widget (promoted to member for drag-reorder wiring)
    QListWidget *m_bookmarkList{nullptr};

    // Managers
    BookmarkManager *m_bookmarkManager{nullptr};
    SettingsManager *m_settingsManager{nullptr};
    LayoutManager   *m_layoutManager{nullptr};   ///< SP-10: layout presets

    // SP-10: layout presets menu (rebuilt whenever presets change)
    QMenu *m_layoutPresetsMenu{nullptr};
};

