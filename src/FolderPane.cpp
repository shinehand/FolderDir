#include "FolderPane.h"

#include <QtWidgets/QTabBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QMenu>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QDrag>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtCore/QDir>
#include <QtCore/QMimeData>

#include "BreadcrumbBar.h"
#include "FileSystemBrowser.h"
#include "BookmarkManager.h"

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
        // The path is base64-encoded so that '|' separators remain unambiguous.
        const QString paneHex  = QString::number(
            reinterpret_cast<quintptr>(m_pane), 16);
        const QString tabPath  = m_pane->tabPaths().value(m_dragTabIndex);
        const QString payload  = paneHex
                               + QLatin1Char('|')
                               + QString::number(m_dragTabIndex)
                               + QLatin1Char('|')
                               + QString::fromLatin1(tabPath.toUtf8().toBase64());

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

    // Breadcrumb address bar
    m_addressBar = new BreadcrumbBar(this);
    m_layout->addWidget(m_addressBar);

    // Tab bar
    m_tabBar = new DraggableTabBar(this, this);
    m_tabBar->setTabsClosable(true);
    m_tabBar->setMovable(true);
    m_tabBar->setExpanding(false);
    m_tabBar->setDocumentMode(true);
    m_tabBar->setUsesScrollButtons(true);
    m_layout->addWidget(m_tabBar);

    // "+" button embedded in tab bar
    QHBoxLayout *tabRow = new QHBoxLayout;
    tabRow->setContentsMargins(0, 0, 0, 0);
    tabRow->setSpacing(0);
    tabRow->addWidget(m_tabBar, 1);

    auto *addTabBtn = new QPushButton(QStringLiteral("+"), this);
    addTabBtn->setFixedSize(24, 24);
    addTabBtn->setToolTip(tr("New Tab (Ctrl+T)"));
    addTabBtn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    tabRow->addWidget(addTabBtn);

    // Replace the single-widget add with the row
    m_layout->removeWidget(m_tabBar);
    auto *tabWidget = new QWidget(this);
    tabWidget->setLayout(tabRow);
    m_layout->addWidget(tabWidget);

    // Stacked widget (one page per tab)
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
void FolderPane::addTabInternal(const QString &path)
{
    auto *browser = new FileSystemBrowser(path, this);
    connect(browser, &FileSystemBrowser::pathChanged,
            this, &FolderPane::onBrowserPathChanged);
    connect(browser, &FileSystemBrowser::selectionChanged,
            this, &FolderPane::onBrowserSelectionChanged);

    m_stack->addWidget(browser);
    const QString label = QDir(path).dirName();
    m_tabBar->addTab(label.isEmpty() ? path : label);
    m_tabBar->setTabToolTip(m_tabBar->count() - 1, path);
    m_tabBar->setCurrentIndex(m_tabBar->count() - 1);
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

void FolderPane::setActiveStyle()
{
    if (m_active) {
        setStyleSheet(QStringLiteral(
            "FolderPane { border: 2px solid palette(highlight); }"));
    } else {
        setStyleSheet(QStringLiteral(
            "FolderPane { border: 2px solid palette(mid); }"));
    }
}

// ──────────────────────────────────────────────────────────────────────────────
void FolderPane::navigateTo(const QString &path)
{
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
    // Remove existing tabs
    while (m_tabBar->count() > 0) closeTab(0);

    for (const QString &p : paths) {
        if (!p.isEmpty()) addTabInternal(p);
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
    if (event->mimeData()->hasFormat(QLatin1String(k_tabDragMime)))
        event->acceptProposedAction();
    else
        QWidget::dragEnterEvent(event);
}

void FolderPane::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(QLatin1String(k_tabDragMime)))
        event->acceptProposedAction();
    else
        QWidget::dragMoveEvent(event);
}

void FolderPane::dropEvent(QDropEvent *event)
{
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
