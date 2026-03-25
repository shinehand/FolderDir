#include "MainWindow.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtGui/QKeyEvent>
#include <QtGui/QCloseEvent>
#include <QtCore/QSettings>
#include <QtCore/QStorageInfo>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>

#include "FolderPane.h"
#include "BookmarkManager.h"
#include "SettingsManager.h"
#include "DriveBar.h"
#include "SearchDialog.h"
#include "PreviewPanel.h"
#include "SettingsDialog.h"
#include "FileSystemBrowser.h"

// ──────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_bookmarkManager = new BookmarkManager(this);
    m_settingsManager = new SettingsManager(this);

    m_settingsManager->applyTheme();

    setupUi();
    setupMenuBar();
    setupToolBar();
    setupDriveBar();
    setupDockWidgets();
    setupStatusBar();

    const int panes = m_settingsManager->paneCount();
    applyLayout(panes);

    setupConnections();

    if (m_settingsManager->restoreSession()) {
        restoreSession();
    }

    resize(1280, 800);
    setWindowTitle(tr("FolderDir"));
    setWindowIcon(QIcon(QStringLiteral(":/icons/app.png")));
}

MainWindow::~MainWindow() = default;

// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *vl = new QVBoxLayout(central);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);

    // Drive bar row
    m_driveBar = new DriveBar(this);
    vl->addWidget(m_driveBar);

    // Splitter tree
    m_hSplitter  = new QSplitter(Qt::Vertical, central);
    m_topSplitter = new QSplitter(Qt::Horizontal, m_hSplitter);
    m_botSplitter = new QSplitter(Qt::Horizontal, m_hSplitter);
    m_hSplitter->addWidget(m_topSplitter);
    m_hSplitter->addWidget(m_botSplitter);

    // Create all four panes
    for (int i = 0; i < 4; ++i) {
        m_panes[i] = new FolderPane(m_bookmarkManager, this);
        connect(m_panes[i], &FolderPane::paneActivated,
                this, &MainWindow::onActivePaneChanged);
        connect(m_panes[i], &FolderPane::selectionChanged,
                this, &MainWindow::onSelectionChanged);
    }

    m_topSplitter->addWidget(m_panes[0]);
    m_topSplitter->addWidget(m_panes[1]);
    m_botSplitter->addWidget(m_panes[2]);
    m_botSplitter->addWidget(m_panes[3]);

    vl->addWidget(m_hSplitter, 1);

    m_activePane = m_panes[0];
}

void MainWindow::setupMenuBar()
{
    // ── File ──────────────────────────────────────────────────────────────
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("New &File"),   this, &MainWindow::onNewFile,
                        QKeySequence(Qt::CTRL | Qt::Key_N));
    fileMenu->addAction(tr("New F&older"), this, &MainWindow::onNewFolder,
                        QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, &MainWindow::onExit,
                        QKeySequence::Quit);

    // ── Edit ──────────────────────────────────────────────────────────────
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(tr("&Copy"),       this, &MainWindow::onCopy,
                        QKeySequence::Copy);
    editMenu->addAction(tr("Cu&t"),        this, &MainWindow::onCut,
                        QKeySequence::Cut);
    editMenu->addAction(tr("&Paste"),      this, &MainWindow::onPaste,
                        QKeySequence::Paste);
    editMenu->addSeparator();
    editMenu->addAction(tr("&Delete"),     this, &MainWindow::onDelete,
                        QKeySequence::Delete);
    editMenu->addAction(tr("&Rename"),     this, &MainWindow::onRename,
                        QKeySequence(Qt::Key_F2));
    editMenu->addSeparator();
    editMenu->addAction(tr("Select &All"), this, &MainWindow::onSelectAll,
                        QKeySequence::SelectAll);
    editMenu->addAction(tr("Copy &Path"),  this, &MainWindow::onCopyPath,
                        QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));

    // ── View ──────────────────────────────────────────────────────────────
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    QMenu *layoutMenu = viewMenu->addMenu(tr("&Layout"));
    layoutMenu->addAction(tr("1 Pane"),
                          this, &MainWindow::onLayout1Pane);
    layoutMenu->addAction(tr("2 Panes (Horizontal)"),
                          this, &MainWindow::onLayout2PanesH);
    layoutMenu->addAction(tr("2 Panes (Vertical)"),
                          this, &MainWindow::onLayout2PanesV);
    layoutMenu->addAction(tr("3 Panes"),
                          this, &MainWindow::onLayout3Panes);
    layoutMenu->addAction(tr("4 Panes"),
                          this, &MainWindow::onLayout4Panes);

    viewMenu->addSeparator();

    m_actHidden = viewMenu->addAction(tr("Show &Hidden Files"));
    m_actHidden->setCheckable(true);
    m_actHidden->setChecked(m_settingsManager->showHiddenFiles());
    m_actHidden->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
    connect(m_actHidden, &QAction::toggled, this, &MainWindow::onToggleHidden);

    m_actPreview = viewMenu->addAction(tr("&Preview Panel"));
    m_actPreview->setCheckable(true);
    m_actPreview->setChecked(false);
    m_actPreview->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    connect(m_actPreview, &QAction::toggled, this, &MainWindow::onTogglePreview);

    m_actBookmarkSidebar = viewMenu->addAction(tr("&Bookmark Sidebar"));
    m_actBookmarkSidebar->setCheckable(true);
    m_actBookmarkSidebar->setChecked(false);
    connect(m_actBookmarkSidebar, &QAction::toggled,
            this, &MainWindow::onToggleBookmarkSidebar);

    // ── Tools ─────────────────────────────────────────────────────────────
    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(tr("&Search…"), this, &MainWindow::onOpenSearch,
                         QKeySequence(Qt::CTRL | Qt::Key_F));
    toolsMenu->addAction(tr("Open &Terminal"), this, &MainWindow::onOpenTerminal);
    toolsMenu->addSeparator();
    toolsMenu->addAction(tr("&Settings…"), this, &MainWindow::onOpenSettings);

    // ── Bookmarks ─────────────────────────────────────────────────────────
    QMenu *bmMenu = menuBar()->addMenu(tr("&Bookmarks"));
    bmMenu->addAction(tr("&Add Bookmark"),
                      this, &MainWindow::onAddBookmark,
                      QKeySequence(Qt::CTRL | Qt::Key_D));
    bmMenu->addSeparator();

    auto rebuildBmMenu = [this, bmMenu]() {
        // Remove dynamic entries beyond the separator
        const auto actions = bmMenu->actions();
        bool pastSep = false;
        for (QAction *a : actions) {
            if (a->isSeparator()) { pastSep = true; continue; }
            if (pastSep) bmMenu->removeAction(a);
        }
        for (const BookmarkEntry &e : m_bookmarkManager->bookmarks()) {
            QAction *a = bmMenu->addAction(
                QIcon::fromTheme(QStringLiteral("folder")), e.name,
                [this, path = e.path]() {
                    if (m_activePane) m_activePane->navigateTo(path);
                });
            (void)a;
        }
    };
    rebuildBmMenu();
    connect(m_bookmarkManager, &BookmarkManager::bookmarksChanged,
            this, rebuildBmMenu);

    // ── Help ──────────────────────────────────────────────────────────────
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About FolderDir"), this, [this]() {
        QMessageBox::about(this, tr("About FolderDir"),
            tr("<b>FolderDir</b> v1.0.0<br>"
               "A multi-pane file manager inspired by Q-Dir.<br>"
               "Built with C++17 and Qt."));
    });
}

void MainWindow::setupToolBar()
{
    m_toolBar = addToolBar(tr("Navigation"));
    m_toolBar->setMovable(false);
    m_toolBar->setIconSize(QSize(20, 20));

    auto *actBack = m_toolBar->addAction(
        QIcon::fromTheme(QStringLiteral("go-previous"), QIcon(QStringLiteral("←"))),
        tr("Back"), this, &MainWindow::onNavigateBack);
    actBack->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Left));

    auto *actFwd = m_toolBar->addAction(
        QIcon::fromTheme(QStringLiteral("go-next"), QIcon(QStringLiteral("→"))),
        tr("Forward"), this, &MainWindow::onNavigateForward);
    actFwd->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Right));

    auto *actUp = m_toolBar->addAction(
        QIcon::fromTheme(QStringLiteral("go-up"), QIcon(QStringLiteral("↑"))),
        tr("Up"), this, &MainWindow::onNavigateUp);
    actUp->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Up));

    m_toolBar->addSeparator();

    m_toolBar->addAction(
        QIcon::fromTheme(QStringLiteral("folder-new")), tr("New Folder"),
        this, &MainWindow::onNewFolder);
    m_toolBar->addAction(
        QIcon::fromTheme(QStringLiteral("edit-delete")), tr("Delete"),
        this, &MainWindow::onDelete);
    m_toolBar->addAction(
        QIcon::fromTheme(QStringLiteral("edit-copy")), tr("Copy"),
        this, &MainWindow::onCopy);
    m_toolBar->addAction(
        QIcon::fromTheme(QStringLiteral("edit-paste")), tr("Paste"),
        this, &MainWindow::onPaste);

    m_toolBar->addSeparator();
    m_toolBar->addAction(
        QIcon::fromTheme(QStringLiteral("system-search")), tr("Search"),
        this, &MainWindow::onOpenSearch);
    m_toolBar->addAction(
        QIcon::fromTheme(QStringLiteral("preferences-system")), tr("Settings"),
        this, &MainWindow::onOpenSettings);
}

void MainWindow::setupDriveBar()
{
    connect(m_driveBar, &DriveBar::driveSelected, this, [this](const QString &root) {
        if (m_activePane) m_activePane->navigateTo(root);
    });
}

void MainWindow::setupDockWidgets()
{
    // ── Bookmark sidebar ──────────────────────────────────────────────────
    m_bookmarkDock = new QDockWidget(tr("Bookmarks"), this);
    m_bookmarkDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    auto *bmList = new QListWidget(m_bookmarkDock);
    m_bookmarkDock->setWidget(bmList);
    addDockWidget(Qt::LeftDockWidgetArea, m_bookmarkDock);
    m_bookmarkDock->hide();

    auto rebuildList = [this, bmList]() {
        bmList->clear();
        for (const BookmarkEntry &e : m_bookmarkManager->bookmarks()) {
            auto *item = new QListWidgetItem(
                QIcon::fromTheme(QStringLiteral("folder")), e.name);
            item->setData(Qt::UserRole, e.path);
            bmList->addItem(item);
        }
    };
    rebuildList();
    connect(m_bookmarkManager, &BookmarkManager::bookmarksChanged,
            this, rebuildList);
    connect(bmList, &QListWidget::itemActivated, this,
            [this](QListWidgetItem *item) {
        if (m_activePane)
            m_activePane->navigateTo(item->data(Qt::UserRole).toString());
    });

    // ── Preview panel ─────────────────────────────────────────────────────
    m_previewDock = new QDockWidget(tr("Preview"), this);
    m_previewDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    m_previewPanel = new PreviewPanel(m_previewDock);
    m_previewDock->setWidget(m_previewPanel);
    addDockWidget(Qt::RightDockWidgetArea, m_previewDock);
    m_previewDock->hide();
}

void MainWindow::setupStatusBar()
{
    m_statusSelection = new QLabel(this);
    m_statusItems     = new QLabel(this);
    m_statusDisk      = new QLabel(this);

    statusBar()->addWidget(m_statusSelection, 2);
    statusBar()->addPermanentWidget(m_statusItems, 1);
    statusBar()->addPermanentWidget(m_statusDisk, 1);

    updateStatusBar();
}

void MainWindow::setupConnections()
{
    // Keep preview panel in sync with the active pane's selection
    for (auto *pane : m_panes) {
        if (!pane) continue;
        connect(pane, &FolderPane::selectionChanged, this, [this, pane]() {
            if (pane != m_activePane) return;
            if (!m_previewDock->isVisible()) return;
            auto *browser = pane->currentBrowser();
            if (!browser) return;
            const QStringList sel = browser->selectedPaths();
            if (sel.size() == 1)
                m_previewPanel->previewFile(sel.first());
            else
                m_previewPanel->clear();
        });
    }
}

// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::applyLayout(int paneCount)
{
    m_paneCount = paneCount;

    // Hide all first
    for (auto *p : m_panes) {
        if (p) p->hide();
    }
    m_botSplitter->hide();
    m_topSplitter->hide();

    switch (paneCount) {
    case 1:
        m_panes[0]->show();
        m_topSplitter->show();
        break;
    case 2:
        m_panes[0]->show();
        m_panes[1]->show();
        m_topSplitter->show();
        break;
    case 3:
        m_panes[0]->show();
        m_panes[1]->show();
        m_panes[2]->show();
        m_topSplitter->show();
        m_botSplitter->show();
        break;
    default: // 4
        for (auto *p : m_panes) if (p) p->show();
        m_topSplitter->show();
        m_botSplitter->show();
        break;
    }

    m_settingsManager->setPaneCount(paneCount);
}

// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::saveSession()
{
    if (!m_settingsManager->restoreSession()) return;
    QSettings *s = m_settingsManager->raw();

    s->beginGroup(QStringLiteral("Session"));
    s->setValue(QStringLiteral("paneCount"), m_paneCount);
    s->setValue(QStringLiteral("geometry"), saveGeometry());
    s->setValue(QStringLiteral("state"), saveState());

    for (int i = 0; i < 4; ++i) {
        if (!m_panes[i]) continue;
        s->beginGroup(QStringLiteral("pane%1").arg(i));
        s->setValue(QStringLiteral("tabs"), m_panes[i]->tabPaths());
        s->setValue(QStringLiteral("currentTab"), m_panes[i]->currentTabIndex());
        s->endGroup();
    }
    s->endGroup();
}

void MainWindow::restoreSession()
{
    QSettings *s = m_settingsManager->raw();
    s->beginGroup(QStringLiteral("Session"));

    const QByteArray geo = s->value(QStringLiteral("geometry")).toByteArray();
    if (!geo.isEmpty()) restoreGeometry(geo);

    const QByteArray state = s->value(QStringLiteral("state")).toByteArray();
    if (!state.isEmpty()) restoreState(state);

    const int panes = s->value(QStringLiteral("paneCount"), 4).toInt();
    applyLayout(panes);

    for (int i = 0; i < 4; ++i) {
        if (!m_panes[i]) continue;
        s->beginGroup(QStringLiteral("pane%1").arg(i));
        const QStringList paths = s->value(QStringLiteral("tabs")).toStringList();
        const int cur = s->value(QStringLiteral("currentTab"), 0).toInt();
        if (!paths.isEmpty())
            m_panes[i]->restoreTabs(paths, cur);
        s->endGroup();
    }
    s->endGroup();
}

// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSession();
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Ctrl+1..4 — activate pane
    if (event->modifiers() == Qt::ControlModifier) {
        const int key = event->key();
        if (key >= Qt::Key_1 && key <= Qt::Key_4) {
            int idx = key - Qt::Key_1;
            if (idx < m_paneCount && m_panes[idx]) {
                m_panes[idx]->setActive(true);
                m_panes[idx]->setFocus();
                onActivePaneChanged(m_panes[idx]);
            }
            event->accept();
            return;
        }
    }
    QMainWindow::keyPressEvent(event);
}

// ──────────────────────────────────────────────────────────────────────────────
FolderPane *MainWindow::activePane() const
{
    return m_activePane ? m_activePane : m_panes[0];
}

// ──────────────────────────────────────────────────────────────────────────────
// Slots
// ──────────────────────────────────────────────────────────────────────────────
void MainWindow::onActivePaneChanged(FolderPane *pane)
{
    if (m_activePane == pane) return;
    m_activePane = pane;
    for (auto *p : m_panes) {
        if (p) p->setActive(p == pane);
    }
    updateStatusBar();
}

void MainWindow::onSelectionChanged()
{
    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    if (!m_activePane) return;
    auto *browser = m_activePane->currentBrowser();
    if (!browser) return;

    const QStringList sel = browser->selectedPaths();
    m_statusSelection->setText(
        tr("%n item(s) selected", "", sel.size()));

    // Disk free
    const QStorageInfo si(m_activePane->currentPath());
    if (si.isValid()) {
        const double freeGb = si.bytesFree() / 1.0e9;
        m_statusDisk->setText(tr("Free: %1 GB").arg(freeGb, 0, 'f', 1));
    }
}

void MainWindow::onNewFile()
{
    if (!m_activePane) return;
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, tr("New File"), tr("File name:"), QLineEdit::Normal, QString(), &ok);
    if (ok && !name.isEmpty())
        m_activePane->currentBrowser()->newFile();
}

void MainWindow::onNewFolder()
{
    if (m_activePane) m_activePane->currentBrowser()->newFolder();
}

void MainWindow::onExit()
{
    close();
}

void MainWindow::onCopy()
{
    if (m_activePane) m_activePane->currentBrowser()->copySelected();
}

void MainWindow::onCut()
{
    if (m_activePane) m_activePane->currentBrowser()->cutSelected();
}

void MainWindow::onPaste()
{
    if (m_activePane) m_activePane->currentBrowser()->pasteHere();
}

void MainWindow::onDelete()
{
    if (m_activePane) m_activePane->currentBrowser()->deleteSelected();
}

void MainWindow::onRename()
{
    if (m_activePane) m_activePane->currentBrowser()->beginRename();
}

void MainWindow::onSelectAll()
{
    if (m_activePane) m_activePane->currentBrowser()->selectAll();
}

void MainWindow::onCopyPath()
{
    if (m_activePane) m_activePane->currentBrowser()->copyPathToClipboard();
}

void MainWindow::onLayout1Pane()    { applyLayout(1); }
void MainWindow::onLayout2PanesH()  { applyLayout(2); }
void MainWindow::onLayout2PanesV()  { applyLayout(2); }
void MainWindow::onLayout3Panes()   { applyLayout(3); }
void MainWindow::onLayout4Panes()   { applyLayout(4); }

void MainWindow::onToggleHidden()
{
    const bool show = m_actHidden->isChecked();
    m_settingsManager->setShowHiddenFiles(show);
    for (auto *p : m_panes) {
        if (p && p->currentBrowser())
            p->currentBrowser()->setShowHidden(show);
    }
}

void MainWindow::onTogglePreview()
{
    m_previewDock->setVisible(m_actPreview->isChecked());
}

void MainWindow::onToggleBookmarkSidebar()
{
    m_bookmarkDock->setVisible(m_actBookmarkSidebar->isChecked());
}

void MainWindow::onOpenSearch()
{
    const QString startPath = m_activePane ? m_activePane->currentPath()
                                           : QDir::homePath();
    auto *dlg = new SearchDialog(startPath, this);
    connect(dlg, &SearchDialog::navigateRequested, this,
            [this](const QString &path) {
        if (m_activePane) m_activePane->navigateTo(QFileInfo(path).absolutePath());
    });
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void MainWindow::onOpenSettings()
{
    SettingsDialog dlg(m_settingsManager, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_settingsManager->applyTheme();
    }
}

void MainWindow::onOpenTerminal()
{
    const QString path = m_activePane ? m_activePane->currentPath()
                                      : QDir::homePath();
#ifdef Q_OS_WIN
    const QString cmd = QStringLiteral("cmd.exe");
    QProcess::startDetached(cmd, {}, path);
#elif defined(Q_OS_MAC)
    QProcess::startDetached(
        QStringLiteral("open"),
        {QStringLiteral("-a"), QStringLiteral("Terminal"), path});
#else
    // Try common terminal emulators
    for (const QString &term :
         {QStringLiteral("x-terminal-emulator"),
          QStringLiteral("gnome-terminal"),
          QStringLiteral("konsole"),
          QStringLiteral("xterm")}) {
        if (QProcess::startDetached(term, {}, path)) break;
    }
#endif
}

void MainWindow::onAddBookmark()
{
    if (!m_activePane) return;
    const QString path = m_activePane->currentPath();
    if (path.isEmpty()) return;
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, tr("Add Bookmark"),
        tr("Bookmark name:"), QLineEdit::Normal,
        QFileInfo(path).fileName(), &ok);
    if (ok && !name.isEmpty())
        m_bookmarkManager->add(path, name);
}

void MainWindow::onNavigateBack()
{
    if (m_activePane) m_activePane->navigateBack();
}

void MainWindow::onNavigateForward()
{
    if (m_activePane) m_activePane->navigateForward();
}

void MainWindow::onNavigateUp()
{
    if (m_activePane) m_activePane->navigateUp();
}
