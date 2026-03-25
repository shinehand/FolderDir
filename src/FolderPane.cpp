#include "FolderPane.h"

#include <QtWidgets/QTabBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QMenu>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtGui/QMouseEvent>
#include <QtCore/QDir>

#include "AddressBar.h"
#include "FileSystemBrowser.h"
#include "BookmarkManager.h"

// ──────────────────────────────────────────────────────────────────────────────
FolderPane::FolderPane(BookmarkManager *bm, QWidget *parent)
    : QWidget(parent)
    , m_bookmarkManager(bm)
{
    setupUi();
    // Default: open home directory
    addTabInternal(QDir::homePath());
}

FolderPane::~FolderPane() = default;

// ──────────────────────────────────────────────────────────────────────────────
void FolderPane::setupUi()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(2, 2, 2, 2);
    m_layout->setSpacing(0);

    // Address bar
    m_addressBar = new AddressBar(this);
    m_layout->addWidget(m_addressBar);

    // Tab bar
    m_tabBar = new QTabBar(this);
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
    connect(m_tabBar, &QTabBar::customContextMenuRequested,
            this, &FolderPane::onTabContextMenu);
    m_tabBar->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(addTabBtn, &QPushButton::clicked,
            this, &FolderPane::onNewTabRequested);
    connect(m_addressBar, &AddressBar::pathCommitted,
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
