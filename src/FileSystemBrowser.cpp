#include "FileSystemBrowser.h"

#include <QtWidgets/QTreeView>
#include <QtWidgets/QListView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtGui/QKeyEvent>
#include <QtGui/QClipboard>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMimeData>
#include <QtGui/QDesktopServices>

#include "FileSystemModel.h"
#include "FileOperations.h"
#include "FileOperationDialog.h"

// ──────────────────────────────────────────────────────────────────────────────
FileSystemBrowser::FileSystemBrowser(const QString &path, QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setPath(path.isEmpty() ? QDir::homePath() : path);
}

FileSystemBrowser::~FileSystemBrowser() = default;

// ──────────────────────────────────────────────────────────────────────────────
void FileSystemBrowser::setupUi()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    // ── Model ────────────────────────────────────────────────────────────
    m_model = new FileSystemModel(this);
    m_model->setReadOnly(false);
    m_model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);

    // ── Details view ─────────────────────────────────────────────────────
    m_detailsView = new QTreeView(this);
    m_detailsView->setModel(m_model);
    m_detailsView->setRootIsDecorated(false);
    m_detailsView->setSortingEnabled(true);
    m_detailsView->sortByColumn(0, Qt::AscendingOrder);
    m_detailsView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_detailsView->setDragEnabled(true);
    m_detailsView->setAcceptDrops(true);
    m_detailsView->setDropIndicatorShown(true);
    m_detailsView->setDragDropMode(QAbstractItemView::DragDrop);
    m_detailsView->setDefaultDropAction(Qt::CopyAction);
    m_detailsView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_detailsView->header()->setSectionsMovable(true);
    m_detailsView->header()->setStretchLastSection(false);
    m_detailsView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    // Hide columns 1-3 (we still have them available but not shown by default)
    // size | type | date
    m_detailsView->setColumnWidth(1, 80);
    m_detailsView->setColumnWidth(2, 100);
    m_detailsView->setColumnWidth(3, 140);

    connect(m_detailsView, &QTreeView::activated,
            this, &FileSystemBrowser::onItemActivated);
    connect(m_detailsView, &QTreeView::customContextMenuRequested,
            this, &FileSystemBrowser::onContextMenu);
    connect(m_detailsView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this, [this]() { emit selectionChanged(); });
    connect(m_detailsView->header(), &QHeaderView::sortIndicatorChanged,
            this, &FileSystemBrowser::onSortIndicatorChanged);

    // ── List view ────────────────────────────────────────────────────────
    m_listView = new QListView(this);
    m_listView->setModel(m_model);
    m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_listView->setDragEnabled(true);
    m_listView->setAcceptDrops(true);
    m_listView->setDropIndicatorShown(true);
    m_listView->setDragDropMode(QAbstractItemView::DragDrop);
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listView->setViewMode(QListView::ListMode);
    m_listView->setWrapping(true);
    m_listView->setResizeMode(QListView::Adjust);
    m_listView->hide();

    connect(m_listView, &QListView::activated,
            this, &FileSystemBrowser::onItemActivated);
    connect(m_listView, &QListView::customContextMenuRequested,
            this, &FileSystemBrowser::onContextMenu);
    connect(m_listView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this, [this]() { emit selectionChanged(); });

    m_layout->addWidget(m_detailsView, 1);
    m_layout->addWidget(m_listView, 1);

    m_activeView = m_detailsView;

    // ── Filter bar (hidden by default) ────────────────────────────────────
    m_filterBar = new QWidget(this);
    m_filterLayout = new QHBoxLayout(m_filterBar);
    m_filterLayout->setContentsMargins(2, 2, 2, 2);
    m_filterLabel = new QLabel(tr("Filter:"), m_filterBar);
    m_filterEdit  = new QLineEdit(m_filterBar);
    m_filterEdit->setPlaceholderText(tr("*.cpp, img_*"));
    m_filterEdit->setClearButtonEnabled(true);
    m_filterLayout->addWidget(m_filterLabel);
    m_filterLayout->addWidget(m_filterEdit, 1);
    m_filterBar->hide();

    connect(m_filterEdit, &QLineEdit::textChanged,
            this, &FileSystemBrowser::onFilterChanged);

    m_layout->addWidget(m_filterBar);
}

// ──────────────────────────────────────────────────────────────────────────────
QString FileSystemBrowser::currentPath() const
{
    return m_currentPath;
}

QAbstractItemView *FileSystemBrowser::view() const
{
    return m_activeView;
}

bool FileSystemBrowser::showHidden() const
{
    return m_model->filter().testFlag(QDir::Hidden);
}

void FileSystemBrowser::setShowHidden(bool show)
{
    QDir::Filters f = m_model->filter();
    if (show)
        f |= QDir::Hidden;
    else
        f &= ~QDir::Hidden;
    m_model->setFilter(f);
}

void FileSystemBrowser::setViewMode(ViewMode mode)
{
    m_viewMode = mode;
    switch (mode) {
    case ViewMode::Details:
        m_detailsView->show();
        m_listView->hide();
        m_activeView = m_detailsView;
        break;
    case ViewMode::List:
        m_detailsView->hide();
        m_listView->show();
        m_listView->setViewMode(QListView::ListMode);
        m_listView->setIconSize(QSize(16, 16));
        m_activeView = m_listView;
        break;
    case ViewMode::Icons:
    case ViewMode::Thumbnails:
        m_detailsView->hide();
        m_listView->show();
        m_listView->setViewMode(QListView::IconMode);
        m_listView->setIconSize(QSize(48, 48));
        m_listView->setGridSize(QSize(80, 80));
        m_activeView = m_listView;
        break;
    }
    if (!m_currentPath.isEmpty()) {
        QModelIndex root = m_model->index(m_currentPath);
        m_detailsView->setRootIndex(root);
        m_listView->setRootIndex(root);
    }
}

// ──────────────────────────────────────────────────────────────────────────────
void FileSystemBrowser::setPath(const QString &path)
{
    if (path.isEmpty()) return;
    const QString cleaned = QDir::cleanPath(path);
    if (!QDir(cleaned).exists()) return;

    if (!m_currentPath.isEmpty() && cleaned != m_currentPath) {
        m_backHistory.append(m_currentPath);
        m_forwardHistory.clear();
    }

    m_currentPath = cleaned;
    const QModelIndex root = m_model->setRootPath(cleaned);
    m_detailsView->setRootIndex(root);
    m_listView->setRootIndex(root);

    emit pathChanged(m_currentPath);
}

void FileSystemBrowser::refresh()
{
    m_model->setRootPath(m_currentPath); // forces re-scan
}

void FileSystemBrowser::navigateUp()
{
    QDir d(m_currentPath);
    if (d.cdUp()) setPath(d.absolutePath());
}

void FileSystemBrowser::navigateBack()
{
    if (m_backHistory.isEmpty()) return;
    m_forwardHistory.prepend(m_currentPath);
    const QString prev = m_backHistory.takeLast();
    m_currentPath.clear(); // prevent re-pushing to back
    setPath(prev);
    // Re-adjust: setPath pushed prev to back again, fix that
    if (!m_backHistory.isEmpty() && m_backHistory.last() == prev)
        m_backHistory.removeLast();
}

void FileSystemBrowser::navigateForward()
{
    if (m_forwardHistory.isEmpty()) return;
    m_backHistory.append(m_currentPath);
    const QString next = m_forwardHistory.takeFirst();
    m_currentPath.clear();
    setPath(next);
    if (!m_forwardHistory.isEmpty() && m_forwardHistory.first() == next)
        m_forwardHistory.removeFirst();
}

// ──────────────────────────────────────────────────────────────────────────────
QStringList FileSystemBrowser::selectedPaths() const
{
    QStringList paths;
    const QModelIndexList sel = m_activeView->selectionModel()->selectedRows();
    paths.reserve(sel.size());
    for (const QModelIndex &idx : sel) {
        paths << m_model->filePath(idx);
    }
    return paths;
}

QString FileSystemBrowser::pathForIndex(const QModelIndex &index) const
{
    return m_model->filePath(index);
}

// ──────────────────────────────────────────────────────────────────────────────
void FileSystemBrowser::beginRename()
{
    const QModelIndexList sel = m_activeView->selectionModel()->selectedRows();
    if (sel.isEmpty()) return;
    m_activeView->edit(sel.first());
}

void FileSystemBrowser::copySelected()
{
    const QStringList paths = selectedPaths();
    if (paths.isEmpty()) return;

    auto *mime = new QMimeData;
    QList<QUrl> urls;
    urls.reserve(paths.size());
    for (const QString &p : paths) urls << QUrl::fromLocalFile(p);
    mime->setUrls(urls);
    mime->setData(QStringLiteral("x-folderdir-cut"), QByteArray("0"));
    QApplication::clipboard()->setMimeData(mime);

    m_clipboard = paths.join(QStringLiteral("\n"));
    m_cutMode = false;
}

void FileSystemBrowser::cutSelected()
{
    copySelected();
    m_cutMode = true;
    const auto *mime = QApplication::clipboard()->mimeData();
    if (mime) {
        auto *newMime = new QMimeData;
        newMime->setUrls(mime->urls());
        newMime->setData(QStringLiteral("x-folderdir-cut"), QByteArray("1"));
        QApplication::clipboard()->setMimeData(newMime);
    }
}

void FileSystemBrowser::pasteHere()
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    if (!mime || !mime->hasUrls()) return;

    QStringList srcs;
    for (const QUrl &u : mime->urls()) {
        if (u.isLocalFile()) srcs << u.toLocalFile();
    }
    if (srcs.isEmpty()) return;

    const bool cut = mime->data(QStringLiteral("x-folderdir-cut")) == QByteArray("1");
    auto *op = new FileOperation(
        cut ? FileOperationKind::Move : FileOperationKind::Copy,
        srcs, m_currentPath);

    auto *dlg = new FileOperationDialog(op, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    op->start();
    dlg->exec();
}

void FileSystemBrowser::deleteSelected(bool permanent)
{
    const QStringList paths = selectedPaths();
    if (paths.isEmpty()) return;

    if (!confirmDelete(paths.size())) return;

    auto *op = new FileOperation(FileOperationKind::Delete, paths, QString());
    auto *dlg = new FileOperationDialog(op, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    op->start();
    dlg->exec();
}

bool FileSystemBrowser::confirmDelete(int count) const
{
    return QMessageBox::question(
        const_cast<FileSystemBrowser *>(this),
        tr("Confirm Delete"),
        tr("Delete %n item(s)?", "", count),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No) == QMessageBox::Yes;
}

void FileSystemBrowser::newFolder()
{
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, tr("New Folder"), tr("Folder name:"),
        QLineEdit::Normal, tr("New Folder"), &ok);
    if (!ok || name.isEmpty()) return;

    QDir dir(m_currentPath);
    if (!dir.mkdir(name)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Could not create folder "%1".").arg(name));
    }
}

void FileSystemBrowser::newFile()
{
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, tr("New File"), tr("File name:"),
        QLineEdit::Normal, tr("New File.txt"), &ok);
    if (!ok || name.isEmpty()) return;

    QFile f(m_currentPath + QDir::separator() + name);
    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Could not create file "%1".").arg(name));
    }
}

void FileSystemBrowser::showProperties()
{
    const QStringList paths = selectedPaths();
    if (paths.isEmpty()) return;

    const QFileInfo fi(paths.first());
    QString info;
    info += tr("Name: %1\n").arg(fi.fileName());
    info += tr("Path: %1\n").arg(fi.absoluteFilePath());
    info += tr("Type: %1\n").arg(fi.isDir() ? tr("Directory") : tr("File"));
    if (!fi.isDir())
        info += tr("Size: %1 bytes\n").arg(fi.size());
    info += tr("Created: %1\n").arg(fi.birthTime().toString(Qt::DefaultLocaleShortDate));
    info += tr("Modified: %1\n").arg(fi.lastModified().toString(Qt::DefaultLocaleShortDate));

    QMessageBox::information(this, tr("Properties"), info);
}

void FileSystemBrowser::copyPathToClipboard()
{
    const QStringList paths = selectedPaths();
    if (paths.isEmpty()) return;
    QApplication::clipboard()->setText(paths.join(QStringLiteral("\n")));
}

void FileSystemBrowser::toggleFilterBar()
{
    m_filterBar->setVisible(!m_filterBar->isVisible());
    if (m_filterBar->isVisible())
        m_filterEdit->setFocus();
}

void FileSystemBrowser::setFilter(const QString &pattern)
{
    m_model->setNameFilterPattern(pattern);
}

void FileSystemBrowser::selectAll()
{
    m_activeView->selectAll();
}

// ──────────────────────────────────────────────────────────────────────────────
// Slots
// ──────────────────────────────────────────────────────────────────────────────
void FileSystemBrowser::onItemActivated(const QModelIndex &index)
{
    const QString path = m_model->filePath(index);
    if (m_model->isDir(index)) {
        setPath(path);
    } else {
        emit fileActivated(path);
        // Open with default application
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}

void FileSystemBrowser::onContextMenu(const QPoint &pos)
{
    const QModelIndex idx = m_activeView->indexAt(pos);
    const bool onItem = idx.isValid();

    QMenu menu(this);

    if (onItem) {
        menu.addAction(QIcon::fromTheme(QStringLiteral("document-open")),
                       tr("Open"), this, [this, idx]() { onItemActivated(idx); });
        menu.addSeparator();
        menu.addAction(tr("Cut"),    this, &FileSystemBrowser::cutSelected);
        menu.addAction(tr("Copy"),   this, &FileSystemBrowser::copySelected);
        menu.addAction(tr("Delete"), this, [this]() { deleteSelected(); });
        menu.addAction(tr("Rename"), this, &FileSystemBrowser::beginRename);
        menu.addSeparator();
        menu.addAction(tr("Copy Path"), this, &FileSystemBrowser::copyPathToClipboard);
        menu.addSeparator();
        menu.addAction(tr("Properties"), this, &FileSystemBrowser::showProperties);
    } else {
        menu.addAction(tr("Paste"),      this, &FileSystemBrowser::pasteHere);
        menu.addAction(tr("New Folder"), this, &FileSystemBrowser::newFolder);
        menu.addAction(tr("New File"),   this, &FileSystemBrowser::newFile);
        menu.addSeparator();
        menu.addAction(tr("Filter…"),    this, &FileSystemBrowser::toggleFilterBar);
        menu.addAction(tr("Refresh"),    this, &FileSystemBrowser::refresh);
    }

    menu.exec(m_activeView->viewport()->mapToGlobal(pos));
}

void FileSystemBrowser::onFilterChanged(const QString &text)
{
    setFilter(text);
}

void FileSystemBrowser::onSortIndicatorChanged(int column, Qt::SortOrder order)
{
    m_model->sort(column, order);
}

// ──────────────────────────────────────────────────────────────────────────────
void FileSystemBrowser::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter: {
        const QModelIndexList sel = m_activeView->selectionModel()->selectedRows();
        if (!sel.isEmpty()) onItemActivated(sel.first());
        break;
    }
    case Qt::Key_F2:
        beginRename();
        break;
    case Qt::Key_Delete:
        deleteSelected(event->modifiers() & Qt::ShiftModifier);
        break;
    case Qt::Key_Backspace:
        navigateUp();
        break;
    case Qt::Key_F5:
        refresh();
        break;
    case Qt::Key_Slash:
    case Qt::Key_Backslash:
        toggleFilterBar();
        break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }
    event->accept();
}

void FileSystemBrowser::applySort()
{
    // Trigger re-sort on the current column
}
