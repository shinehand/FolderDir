#include "FolderPane.h"

#include <QtWidgets/QTabBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QMenu>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QDrag>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragLeaveEvent>
#include <QtGui/QDropEvent>
#include <QtWidgets/QFileIconProvider>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QMimeData>

#include "BreadcrumbBar.h"
#include "FileSystemBrowser.h"
#include "BookmarkManager.h"
#include "SettingsManager.h"

static const char *k_tabDragMime = "application/x-folderdir-tabdrag";

// ──────────────────────────────────────────────────────────────────────────────
/**
 * @brief DraggableTabBar — QTabBar that supports dragging a tab to another pane.
 *
 * Drags are encoded as "pane_hex|tab_index|tab_path" in a custom MIME type.
 * When the drag completes with Qt::MoveAction the source tab is removed.
 */
class DraggableTabBar : public QTabBar
{
public:
    explicit DraggableTabBar(FolderPane *pane, QWidget *parent = nullptr)
        : QTabBar(parent), m_pane(pane) {}

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            m_dragStart    = event->pos();
            m_dragTabIndex = tabAt(event->pos());
        }
        QTabBar::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (m_dragTabIndex < 0) {
            QTabBar::mouseMoveEvent(event);
            return;
        }
        if (!(event->buttons() & Qt::LeftButton)) {
            QTabBar::mouseMoveEvent(event);
            return;
        }

        const QPoint delta = event->pos() - m_dragStart;
        if (delta.manhattanLength() < QApplication::startDragDistance()) {
            QTabBar::mouseMoveEvent(event);
            return;
        }

        // Only start cross-pane drag when the cursor leaves the tab bar
        if (rect().contains(event->pos())) {
            QTabBar::mouseMoveEvent(event);
            return;
        }

        // Build MIME payload: "pane_hex|tab_index|base64(tab_path)"
        // Format: three '|'-delimited fields.  pane_hex and tab_index are plain
        // ASCII values; tab_path is base64-encoded so that any '|' characters
        // inside the path do not break the field delimiter.
        const QString paneHex  = QString::number(
            reinterpret_cast<quintptr>(m_pane), 16);
        const QString tabPath  = m_pane->tabPaths().value(m_dragTabIndex);
        const QString payload  = paneHex
                               + QLatin1Char('|')
                               + QString::number(m_dragTabIndex)
                               + QLatin1Char('|')
                               + QString::fromLatin1(tabPath.toUtf8().toBase64()); // base64 for '|' safety

        auto *mime = new QMimeData;
        mime->setData(QLatin1String(k_tabDragMime), payload.toUtf8());

        auto *drag = new QDrag(this);
        drag->setMimeData(mime);
        drag->setPixmap(grab(tabRect(m_dragTabIndex)));

        const int srcIndex = m_dragTabIndex;
        m_dragTabIndex = -1; // reset before exec blocks

        const Qt::DropAction action = drag->exec(Qt::MoveAction);
        if (action == Qt::MoveAction) {
            // Source tab was accepted by a different pane — remove it here
            m_pane->closeTab(srcIndex);
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        m_dragTabIndex = -1;
        QTabBar::mouseReleaseEvent(event);
    }

private:
    FolderPane *m_pane{nullptr};
    QPoint      m_dragStart;
    int         m_dragTabIndex{-1};
};

// Static registry of all live FolderPane instances
QSet<FolderPane *> FolderPane::s_liveInstances;

// ──────────────────────────────────────────────────────────────────────────────
FolderPane::FolderPane(BookmarkManager *bm, QWidget *parent)
    : QWidget(parent)
    , m_bookmarkManager(bm)
{
    s_liveInstances.insert(this);
    setAcceptDrops(true);
    setupUi();
    // Default: open home directory
    addTabInternal(QDir::homePath());
}

FolderPane::~FolderPane()
{
    s_liveInstances.remove(this);
}

// ──────────────────────────────────────────────────────────────────────────────
void FolderPane::setupUi()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(2, 2, 2, 2);
    m_layout->setSpacing(0);

    // ── Tab bar row (FIRST — matches Q-Dir visual layout: tabs at top) ────
    m_tabBar = new DraggableTabBar(this, this);
    m_tabBar->setTabsClosable(false);   // managed dynamically via updateTabCloseButtons()
    m_tabBar->setMovable(true);
    m_tabBar->setExpanding(false);
    m_tabBar->setDocumentMode(true);
    m_tabBar->setUsesScrollButtons(true);

    // "+" new-tab button sits to the right of the tab bar
    QHBoxLayout *tabRow = new QHBoxLayout;
    tabRow->setContentsMargins(0, 0, 0, 0);
    tabRow->setSpacing(0);
    tabRow->addWidget(m_tabBar, 1);

    auto *addTabBtn = new QPushButton(QStringLiteral("+"), this);
    addTabBtn->setFixedSize(24, 24);
    addTabBtn->setToolTip(tr("New Tab (Ctrl+T)"));
    // Do NOT set a shortcut here — Ctrl+T is already handled by MainWindow
    // to avoid per-pane shortcut conflicts.
    tabRow->addWidget(addTabBtn);

    auto *tabWidget = new QWidget(this);
    tabWidget->setLayout(tabRow);
    m_layout->addWidget(tabWidget);

    // ── Breadcrumb / view-mode row (SECOND — below the tab bar) ──────────
    QHBoxLayout *addrRow = new QHBoxLayout;
    addrRow->setContentsMargins(0, 0, 0, 0);
    addrRow->setSpacing(2);

    m_addressBar = new BreadcrumbBar(this);
    addrRow->addWidget(m_addressBar, 1);

    // Per-pane view-mode selector (UX-B01): small dropdown button on the right
    m_viewModeBtn = new QToolButton(this);
    m_viewModeBtn->setToolTip(tr("View Mode"));
    m_viewModeBtn->setPopupMode(QToolButton::InstantPopup);
    m_viewModeBtn->setFixedSize(28, 24);

    auto *vmMenu = new QMenu(m_viewModeBtn);
    auto makeVmAct = [&](const QString &label, ViewMode mode) {
        auto *act = vmMenu->addAction(label);
        act->setCheckable(true);
        connect(act, &QAction::triggered, this, [this, mode]() { setViewMode(mode); });
        return act;
    };
    auto *actVmDetails    = makeVmAct(tr("Details"),    ViewMode::Details);
    auto *actVmList       = makeVmAct(tr("List"),       ViewMode::List);
    auto *actVmIcons      = makeVmAct(tr("Icons"),      ViewMode::Icons);
    auto *actVmThumbnails = makeVmAct(tr("Thumbnails"), ViewMode::Thumbnails);
    actVmDetails->setChecked(true); // default

    // Keep a reference so syncViewModeButton can update checked state
    vmMenu->setProperty("actDetails",    QVariant::fromValue(actVmDetails));
    vmMenu->setProperty("actList",       QVariant::fromValue(actVmList));
    vmMenu->setProperty("actIcons",      QVariant::fromValue(actVmIcons));
    vmMenu->setProperty("actThumbnails", QVariant::fromValue(actVmThumbnails));

    m_viewModeBtn->setMenu(vmMenu);
    syncViewModeButton(ViewMode::Details);
    addrRow->addWidget(m_viewModeBtn);

    auto *addrWidget = new QWidget(this);
    addrWidget->setLayout(addrRow);
    m_layout->addWidget(addrWidget);

    // ── Stacked file-browser content (THIRD) ─────────────────────────────
    m_stack = new QStackedWidget(this);
    m_layout->addWidget(m_stack, 1);

    // Connections
    connect(m_tabBar, &QTabBar::currentChanged,
            this, &FolderPane::onTabChanged);
    connect(m_tabBar, &QTabBar::tabCloseRequested,
            this, &FolderPane::onTabCloseRequested);
    connect(m_tabBar, &QTabBar::tabBarDoubleClicked,
            this, &FolderPane::onTabDoubleClicked);
    connect(m_tabBar, &QTabBar::customContextMenuRequested,
            this, &FolderPane::onTabContextMenu);
    m_tabBar->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(addTabBtn, &QPushButton::clicked,
            this, &FolderPane::onNewTabRequested);
    connect(m_addressBar, &BreadcrumbBar::pathCommitted,
            this, &FolderPane::onAddressBarCommit);

    setActiveStyle();
}

// ──────────────────────────────────────────────────────────────────────────────
void FolderPane::setSettingsManager(SettingsManager *mgr)
{
    m_settingsMgr = mgr;
    // Propagate to all existing browsers
    for (int i = 0; i < m_stack->count(); ++i) {
        if (auto *b = browserAt(i))
            b->setSettingsManager(mgr);
    }
}

// ──────────────────────────────────────────────────────────────────────────────
void FolderPane::addTabInternal(const QString &path)
{
    auto *browser = new FileSystemBrowser(path, this);
    connect(browser, &FileSystemBrowser::pathChanged,
            this, &FolderPane::onBrowserPathChanged);
    connect(browser, &FileSystemBrowser::selectionChanged,
            this, &FolderPane::onBrowserSelectionChanged);

    // 이 패널의 현재 뷰 모드를 새 탭에도 적용 (UX-B01)
    browser->setViewMode(m_viewMode);
    // 설정 주입 (GAP-001/002)
    if (m_settingsMgr)
        browser->setSettingsManager(m_settingsMgr);

    m_stack->addWidget(browser);
    const QString label = QDir(path).dirName();
    const int tabIdx = m_tabBar->count();
    m_tabBar->addTab(label.isEmpty() ? path : label);
    m_tabBar->setTabToolTip(tabIdx, path);

    // Folder icon in tab (UX-B03)
    QFileIconProvider iconProvider;
    m_tabBar->setTabIcon(tabIdx, iconProvider.icon(QFileInfo(path)));

    m_tabBar->setCurrentIndex(tabIdx);
    updateTabCloseButtons();
}

FileSystemBrowser *FolderPane::browserAt(int index) const
{
    if (index < 0 || index >= m_stack->count()) return nullptr;
    return qobject_cast<FileSystemBrowser *>(m_stack->widget(index));
}

FileSystemBrowser *FolderPane::currentBrowser() const
{
    return browserAt(m_tabBar->currentIndex());
}

// ──────────────────────────────────────────────────────────────────────────────
QString FolderPane::currentPath() const
{
    auto *b = currentBrowser();
    return b ? b->currentPath() : QString();
}

void FolderPane::setActive(bool active)
{
    m_active = active;
    setActiveStyle();
}

void FolderPane::setLocked(bool locked)
{
    m_locked = locked;
    setActiveStyle();
}

void FolderPane::setActiveStyle()
{
    if (m_dropHighlight) {
        // 파일 드롭 대상으로 하이라이트 중 (UX-B04)
        setStyleSheet(QStringLiteral(
            "FolderPane { border: 2px dashed palette(highlight); background: palette(alternateBase); }"));
    } else if (m_locked) {
        // SP-9: locked pane — orange border to indicate navigation is prevented
        setStyleSheet(QStringLiteral(
            "FolderPane { border: 2px solid orange; }"));
    } else if (m_active) {
        setStyleSheet(QStringLiteral(
            "FolderPane { border: 2px solid palette(highlight); }"));
    } else {
        setStyleSheet(QStringLiteral(
            "FolderPane { border: 2px solid palette(mid); }"));
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// UX-B01: 패널별 뷰 모드 — 이 패널의 현재 브라우저에만 적용
void FolderPane::setViewMode(ViewMode mode)
{
    m_viewMode = mode;
    if (auto *b = currentBrowser())
        b->setViewMode(mode);
    syncViewModeButton(mode);
}

void FolderPane::syncViewModeButton(ViewMode mode)
{
    if (!m_viewModeBtn) return;
    struct { ViewMode mode; const char *label; const char *tooltip; } labels[] = {
        { ViewMode::Details,    "≡", QT_TR_NOOP("Details")    },
        { ViewMode::List,       "☰", QT_TR_NOOP("List")       },
        { ViewMode::Icons,      "⊞", QT_TR_NOOP("Icons")      },
        { ViewMode::Thumbnails, "▦", QT_TR_NOOP("Thumbnails") },
    };
    for (const auto &l : labels) {
        if (l.mode == mode) {
            m_viewModeBtn->setText(QLatin1String(l.label));
            // 접근성: 현재 선택된 뷰 모드를 툴팁으로도 제공
            m_viewModeBtn->setToolTip(tr("View Mode: %1").arg(tr(l.tooltip)));
            m_viewModeBtn->setAccessibleName(tr("View Mode: %1").arg(tr(l.tooltip)));
            break;
        }
    }

    // 체크 상태 동기화
    auto *menu = m_viewModeBtn->menu();
    if (!menu) return;
    auto syncAct = [&](const char *propName, ViewMode m) {
        if (auto *act = menu->property(propName).value<QAction *>())
            act->setChecked(m == mode);
    };
    syncAct("actDetails",    ViewMode::Details);
    syncAct("actList",       ViewMode::List);
    syncAct("actIcons",      ViewMode::Icons);
    syncAct("actThumbnails", ViewMode::Thumbnails);
}

void FolderPane::updateTabCloseButtons()
{
    // Show close buttons only when there are 2 or more tabs, matching
    // browser convention: a lone tab's close button would do nothing and
    // is confusing.
    m_tabBar->setTabsClosable(m_tabBar->count() > 1);
}

// ──────────────────────────────────────────────────────────────────────────────
void FolderPane::navigateTo(const QString &path)
{
    if (m_locked) return;
    auto *b = currentBrowser();
    if (b) b->setPath(path);
}

void FolderPane::newTab(const QString &path)
{
    addTabInternal(path.isEmpty() ? QDir::homePath() : path);
}

void FolderPane::closeTab(int index)
{
    if (m_tabBar->count() <= 1) return; // keep at least one tab
    auto *w = m_stack->widget(index);
    m_stack->removeWidget(w);
    w->deleteLater();
    m_tabBar->removeTab(index);
    updateTabCloseButtons();
}

void FolderPane::closeCurrentTab()
{
    closeTab(m_tabBar->currentIndex());
}

void FolderPane::navigateBack()
{
    if (auto *b = currentBrowser()) b->navigateBack();
}

void FolderPane::navigateForward()
{
    if (auto *b = currentBrowser()) b->navigateForward();
}

void FolderPane::navigateUp()
{
    if (auto *b = currentBrowser()) b->navigateUp();
}

void FolderPane::refresh()
{
    if (auto *b = currentBrowser()) b->refresh();
}

void FolderPane::nextTab()
{
    const int count = m_tabBar->count();
    if (count < 2) return;
    const int next = (m_tabBar->currentIndex() + 1) % count;
    m_tabBar->setCurrentIndex(next);
}

// ──────────────────────────────────────────────────────────────────────────────
QStringList FolderPane::tabPaths() const
{
    QStringList paths;
    for (int i = 0; i < m_stack->count(); ++i) {
        auto *b = browserAt(i);
        if (b) paths << b->currentPath();
    }
    return paths;
}

int FolderPane::currentTabIndex() const
{
    return m_tabBar->currentIndex();
}

void FolderPane::restoreTabs(const QStringList &paths, int activeIndex)
{
    if (paths.isEmpty()) return;

    // closeTab() refuses to close the very last tab, so close down to 1 tab first.
    while (m_tabBar->count() > 1)
        closeTab(m_tabBar->count() - 1);

    // Reuse the single remaining tab for the first restored path.
    if (auto *b = currentBrowser(); b && !paths.first().isEmpty())
        b->setPath(paths.first());

    // Add the remaining paths as new tabs.
    for (int i = 1; i < paths.size(); ++i) {
        if (!paths.at(i).isEmpty())
            addTabInternal(paths.at(i));
    }

    if (activeIndex >= 0 && activeIndex < m_tabBar->count())
        m_tabBar->setCurrentIndex(activeIndex);
}

// ──────────────────────────────────────────────────────────────────────────────
// Slots
// ──────────────────────────────────────────────────────────────────────────────
void FolderPane::onTabChanged(int index)
{
    m_stack->setCurrentIndex(index);
    syncAddressBar();
}

void FolderPane::onTabCloseRequested(int index)
{
    closeTab(index);
}

void FolderPane::onAddressBarCommit(const QString &path)
{
    navigateTo(path);
}

void FolderPane::onBrowserPathChanged(const QString &path)
{
    syncAddressBar();
    const int idx = m_tabBar->currentIndex();
    const QString label = QDir(path).dirName();
    m_tabBar->setTabText(idx, label.isEmpty() ? path : label);
    m_tabBar->setTabToolTip(idx, path);

    // Keep tab icon in sync with the new path (UX-B03)
    QFileIconProvider iconProvider;
    m_tabBar->setTabIcon(idx, iconProvider.icon(QFileInfo(path)));

    emit pathChanged(path);
}

void FolderPane::onBrowserSelectionChanged()
{
    emit selectionChanged();
}

void FolderPane::onNewTabRequested()
{
    newTab(currentPath());
}

void FolderPane::onTabContextMenu(const QPoint &pos)
{
    const int idx = m_tabBar->tabAt(pos);
    if (idx < 0) return;

    QMenu menu(this);
    menu.addAction(tr("New Tab"), this, &FolderPane::onNewTabRequested);
    menu.addAction(tr("Close Tab"), this, [this, idx]() { closeTab(idx); });
    menu.addAction(tr("Rename Tab…"), this, [this, idx]() {
        bool ok = false;
        const QString current = m_tabBar->tabText(idx);
        const QString name = QInputDialog::getText(
            this, tr("Rename Tab"), tr("Tab name:"),
            QLineEdit::Normal, current, &ok);
        if (ok && !name.isEmpty())
            m_tabBar->setTabText(idx, name);
    });
    menu.addSeparator();

    if (m_bookmarkManager) {
        auto *b = browserAt(idx);
        if (b) {
            const QString path = b->currentPath();
            menu.addAction(tr("Add to Bookmarks"), this, [this, path]() {
                m_bookmarkManager->add(path);
            });
        }
    }
    menu.exec(m_tabBar->mapToGlobal(pos));
}

void FolderPane::onTabDoubleClicked(int index)
{
    if (index < 0) return;
    bool ok = false;
    const QString current = m_tabBar->tabText(index);
    const QString name = QInputDialog::getText(
        this, tr("Rename Tab"), tr("Tab name:"),
        QLineEdit::Normal, current, &ok);
    if (ok && !name.isEmpty())
        m_tabBar->setTabText(index, name);
}

void FolderPane::syncAddressBar()
{
    auto *b = currentBrowser();
    if (b) m_addressBar->setPath(b->currentPath());
}

// ──────────────────────────────────────────────────────────────────────────────
void FolderPane::mousePressEvent(QMouseEvent *event)
{
    emit paneActivated(this);
    QWidget::mousePressEvent(event);
}

void FolderPane::focusInEvent(QFocusEvent *event)
{
    emit paneActivated(this);
    QWidget::focusInEvent(event);
}

// ──────────────────────────────────────────────────────────────────────────────
void FolderPane::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mime = event->mimeData();
    if (mime->hasFormat(QLatin1String(k_tabDragMime))) {
        // 탭 드래그: 수락 + 하이라이트 (UX-B04)
        m_dropHighlight = true;
        setActiveStyle();
        event->acceptProposedAction();
    } else if (mime->hasUrls()) {
        // 파일/폴더 드래그: 수락 + 하이라이트 (UX-B04)
        m_dropHighlight = true;
        setActiveStyle();
        event->acceptProposedAction();
    } else {
        QWidget::dragEnterEvent(event);
    }
}

void FolderPane::dragMoveEvent(QDragMoveEvent *event)
{
    const QMimeData *mime = event->mimeData();
    if (mime->hasFormat(QLatin1String(k_tabDragMime)) || mime->hasUrls())
        event->acceptProposedAction();
    else
        QWidget::dragMoveEvent(event);
}

void FolderPane::dragLeaveEvent(QDragLeaveEvent *event)
{
    // 드래그가 패널을 벗어나면 하이라이트 해제 (UX-B04)
    m_dropHighlight = false;
    setActiveStyle();
    QWidget::dragLeaveEvent(event);
}

void FolderPane::dropEvent(QDropEvent *event)
{
    // 하이라이트 항상 해제
    m_dropHighlight = false;
    setActiveStyle();

    if (!event->mimeData()->hasFormat(QLatin1String(k_tabDragMime))) {
        QWidget::dropEvent(event);
        return;
    }

    const QString payload = QString::fromUtf8(
        event->mimeData()->data(QLatin1String(k_tabDragMime)));
    const QStringList parts = payload.split(QLatin1Char('|'));
    if (parts.size() < 3) return;

    bool ok = false;
    const quintptr panePtr = parts.at(0).toULongLong(&ok, 16);
    if (!ok) return;
    auto *sourcePane = reinterpret_cast<FolderPane *>(panePtr);

    // Validate the pointer is still a live pane before dereferencing
    if (!s_liveInstances.contains(sourcePane)) return;

    if (sourcePane == this) return; // same-pane moves handled by QTabBar

    // Path was base64-encoded in DraggableTabBar to keep the '|' separator unambiguous
    const QString tabPath = QString::fromUtf8(
        QByteArray::fromBase64(parts.at(2).toLatin1()));

    newTab(tabPath);
    // Signal MoveAction so the source removes the tab
    event->setDropAction(Qt::MoveAction);
    event->accept();
}
