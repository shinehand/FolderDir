#include "FolderTreePanel.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHeaderView>
#include <QtGui/QFileSystemModel>
#include <QtGui/QFileIconProvider>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QFileInfo>

// ──────────────────────────────────────────────────────────────────────────────
FolderTreePanel::FolderTreePanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

// ──────────────────────────────────────────────────────────────────────────────
void FolderTreePanel::setupUi()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    // ── Quick-access pinned section (UX-B06) ─────────────────────────────
    auto *qaLabel = new QLabel(tr("Quick Access"), this);
    qaLabel->setStyleSheet(QStringLiteral(
        "QLabel { font-size: 10px; font-weight: bold; color: palette(mid);"
        "         padding: 3px 4px 2px 4px; background: palette(window); }"));
    m_layout->addWidget(qaLabel);

    m_quickAccess = new QListWidget(this);
    m_quickAccess->setFrameShape(QFrame::NoFrame);
    m_quickAccess->setIconSize(QSize(16, 16));
    m_quickAccess->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_quickAccess->setSpacing(0);
    m_quickAccess->setFixedHeight(0); // will be set by buildQuickAccess()
    m_layout->addWidget(m_quickAccess);

    buildQuickAccess();

    connect(m_quickAccess, &QListWidget::itemClicked,
            this, &FolderTreePanel::onQuickAccessClicked);

    // ── Separator label ───────────────────────────────────────────────────
    auto *sepLabel = new QLabel(tr("Folders"), this);
    sepLabel->setStyleSheet(QStringLiteral(
        "QLabel { font-size: 10px; font-weight: bold; color: palette(mid);"
        "         padding: 3px 4px 2px 4px; background: palette(window); }"));
    m_layout->addWidget(sepLabel);

    // ── Full file-system tree ─────────────────────────────────────────────
    m_model = new QFileSystemModel(this);
    m_model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Drives);
    m_model->setRootPath(QString()); // Monitor entire file system

    m_treeView = new QTreeView(this);
    m_treeView->setModel(m_model);
    m_treeView->setRootIndex(QModelIndex()); // invalid index = show all top-level items

    // Hide all columns except "Name" (column 0)
    m_treeView->setHeaderHidden(true);
    for (int col = 1; col < m_model->columnCount(); ++col)
        m_treeView->hideColumn(col);

    m_treeView->setAnimated(true);
    m_treeView->setIndentation(16);
    m_treeView->setSortingEnabled(false);
    m_treeView->setExpandsOnDoubleClick(true);
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_treeView->setDragEnabled(false);

    connect(m_treeView, &QTreeView::clicked,
            this, &FolderTreePanel::onClicked);

    m_layout->addWidget(m_treeView, 1);
}

// ──────────────────────────────────────────────────────────────────────────────
void FolderTreePanel::buildQuickAccess()
{
    m_quickAccess->clear();

    // Ordered list of well-known locations with display labels
    static const struct { QStandardPaths::StandardLocation loc; const char *label; } kLocations[] = {
        { QStandardPaths::HomeLocation,      "Home"      },
        { QStandardPaths::DesktopLocation,   "Desktop"   },
        { QStandardPaths::DocumentsLocation, "Documents" },
        { QStandardPaths::DownloadLocation,  "Downloads" },
        { QStandardPaths::MusicLocation,     "Music"     },
        { QStandardPaths::PicturesLocation,  "Pictures"  },
        { QStandardPaths::MoviesLocation,    "Videos"    },
    };

    QFileIconProvider iconProvider;
    int validCount = 0;
    for (const auto &loc : kLocations) {
        const QString path = QStandardPaths::writableLocation(loc.loc);
        if (path.isEmpty() || !QFileInfo::exists(path)) continue;

        const QIcon icon = iconProvider.icon(QFileInfo(path));
        auto *item = new QListWidgetItem(icon, tr(loc.label));
        item->setData(Qt::UserRole, path);
        item->setToolTip(path);
        m_quickAccess->addItem(item);
        ++validCount;
    }

    // Resize the list to show all items without a scrollbar.
    // Use the actual per-row height hint for DPI/style independence.
    if (validCount > 0) {
        const int rowH = m_quickAccess->sizeHintForRow(0);
        const int frameExtra = 2 * m_quickAccess->frameWidth();
        m_quickAccess->setFixedHeight(validCount * rowH + frameExtra);
    } else {
        m_quickAccess->setFixedHeight(0);
    }
}

// ──────────────────────────────────────────────────────────────────────────────
void FolderTreePanel::setActivePath(const QString &path)
{
    if (path.isEmpty() || m_syncInProgress) return;

    const QModelIndex idx = m_model->index(path);
    if (!idx.isValid()) return;

    m_syncInProgress = true;

    // Expand all parent nodes, then scroll to and select the target
    m_treeView->setCurrentIndex(idx);
    m_treeView->scrollTo(idx, QAbstractItemView::PositionAtCenter);

    // Expand the item itself so its children are visible
    m_treeView->expand(idx);

    // Deselect any quick-access item (the tree is the authoritative location)
    m_quickAccess->clearSelection();

    m_syncInProgress = false;
}

// ──────────────────────────────────────────────────────────────────────────────
void FolderTreePanel::onClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    const QString path = m_model->filePath(index);
    if (!path.isEmpty()) {
        m_quickAccess->clearSelection(); // deselect quick-access on tree click
        emit navigateRequested(path);
    }
}

void FolderTreePanel::onQuickAccessClicked(QListWidgetItem *item)
{
    if (!item) return;
    const QString path = item->data(Qt::UserRole).toString();
    if (!path.isEmpty()) {
        m_treeView->clearSelection(); // deselect tree on quick-access click
        emit navigateRequested(path);
    }
}
