#include "FolderTreePanel.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QFileSystemModel>
#include <QtCore/QDir>

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

    // Model — show only directories on all drives / root
    m_model = new QFileSystemModel(this);
    m_model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Drives);
    m_model->setRootPath(QString()); // Monitor entire file system

    // Tree view
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

    m_syncInProgress = false;
}

// ──────────────────────────────────────────────────────────────────────────────
void FolderTreePanel::onClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    const QString path = m_model->filePath(index);
    if (!path.isEmpty())
        emit navigateRequested(path);
}
