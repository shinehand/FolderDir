#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QAction>
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

/**
 * @brief MainWindow — top-level application window.
 *
 * Hosts up to four FolderPane instances arranged in a 2×2 grid of
 * QSplitters, plus a menu bar, toolbar, drive bar, status bar, and
 * optional side panels (bookmarks, preview).
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

    // View menu
    void onLayout1Pane();
    void onLayout2PanesH();
    void onLayout2PanesV();
    void onLayout3Panes();
    void onLayout4Panes();
    void onToggleHidden();
    void onTogglePreview();
    void onToggleBookmarkSidebar();

    // Tools menu
    void onOpenSearch();
    void onOpenSettings();
    void onOpenTerminal();

    // Bookmarks menu
    void onAddBookmark();

    // Navigation
    void onNavigateBack();
    void onNavigateForward();
    void onNavigateUp();

    // Status
    void onActivePaneChanged(FolderPane *pane);
    void onSelectionChanged();
    void updateStatusBar();

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

    PreviewPanel *m_previewPanel{nullptr};

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

    // Managers
    BookmarkManager *m_bookmarkManager{nullptr};
    SettingsManager *m_settingsManager{nullptr};
};
