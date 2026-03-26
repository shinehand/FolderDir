#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSplitter>
#include <QtGui/QAction>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDockWidget>
#include <QtCore/QSettings>
#include <array>

class FolderPane;
class BookmarkManager;
class SettingsManager;
class DriveBar;
class SearchDialog;
class PreviewPanel;
class FolderTreePanel;

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

    // Managers
    BookmarkManager *m_bookmarkManager{nullptr};
    SettingsManager *m_settingsManager{nullptr};
};
