#include "WorkspaceWidget.h"

#include <QtWidgets/QVBoxLayout>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include "FolderPane.h"
#include "BookmarkManager.h"
#include "SettingsManager.h"

// ─────────────────────────────────────────────────────────────────────────────
WorkspaceWidget::WorkspaceWidget(BookmarkManager *bm, SettingsManager *sm,
                                 QWidget *parent)
    : QWidget(parent)
    , m_bookmarkManager(bm)
    , m_settingsManager(sm)
{
    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);

    // 2×2 splitter tree
    m_hSplitter   = new QSplitter(Qt::Vertical,   this);
    m_topSplitter = new QSplitter(Qt::Horizontal, m_hSplitter);
    m_botSplitter = new QSplitter(Qt::Horizontal, m_hSplitter);
    m_hSplitter->addWidget(m_topSplitter);
    m_hSplitter->addWidget(m_botSplitter);
    vl->addWidget(m_hSplitter, 1);

    createPanes();
    applyLayout(1); // default: single pane
}

// ─────────────────────────────────────────────────────────────────────────────
void WorkspaceWidget::createPanes()
{
    for (int i = 0; i < 4; ++i) {
        m_panes[i] = new FolderPane(m_bookmarkManager, this);
        m_panes[i]->setSettingsManager(m_settingsManager);

        connect(m_panes[i], &FolderPane::paneActivated,
                this, &WorkspaceWidget::onChildPaneActivated);
        connect(m_panes[i], &FolderPane::selectionChanged,
                this, &WorkspaceWidget::selectionChanged);
        connect(m_panes[i], &FolderPane::pathChanged,
                this, [this, i](const QString &path) {
                    onChildPathChanged(path, m_panes[i]);
                });
    }

    m_topSplitter->addWidget(m_panes[0]);
    m_topSplitter->addWidget(m_panes[1]);
    m_botSplitter->addWidget(m_panes[2]);
    m_botSplitter->addWidget(m_panes[3]);

    m_activePane = m_panes[0];
    m_panes[0]->setActive(true);
}

// ─────────────────────────────────────────────────────────────────────────────
void WorkspaceWidget::applyLayout(int count)
{
    m_paneCount = qBound(1, count, 4);

    for (auto *p : m_panes) if (p) p->hide();
    m_botSplitter->hide();
    m_topSplitter->hide();

    switch (m_paneCount) {
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

    // If the active pane is now hidden, fall back to pane[0]
    if (m_activePane && !m_activePane->isVisible()) {
        setActivePane(m_panes[0]);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
FolderPane *WorkspaceWidget::pane(int index) const
{
    if (index < 0 || index >= 4) return nullptr;
    return m_panes[index];
}

void WorkspaceWidget::setActivePane(FolderPane *p)
{
    if (m_activePane == p) return;
    m_activePane = p;
    for (auto *pane : m_panes)
        if (pane) pane->setActive(pane == p);
}

// ─────────────────────────────────────────────────────────────────────────────
QString WorkspaceWidget::otherPanePath() const
{
    for (int i = 0; i < m_paneCount; ++i) {
        if (m_panes[i] && m_panes[i] != m_activePane && m_panes[i]->isVisible())
            return m_panes[i]->currentPath();
    }
    return m_activePane ? m_activePane->currentPath() : QDir::homePath();
}

// ─────────────────────────────────────────────────────────────────────────────
// Private slots
// ─────────────────────────────────────────────────────────────────────────────
void WorkspaceWidget::onChildPaneActivated(FolderPane *pane)
{
    if (m_activePane == pane) return;
    setActivePane(pane);
    emit paneActivated(pane);
}

void WorkspaceWidget::onChildPathChanged(const QString &path, FolderPane *srcPane)
{
    // Only forward path changes from the active pane
    if (srcPane != m_activePane) return;

    // SP-9: pane sync within this workspace
    if (m_paneSyncEnabled) {
        for (int i = 0; i < m_paneCount; ++i) {
            if (m_panes[i] && m_panes[i] != srcPane && m_panes[i]->isVisible())
                m_panes[i]->navigateTo(path);
        }
    }

    emit pathChanged(path);
}

// ─────────────────────────────────────────────────────────────────────────────
// Session persistence
// ─────────────────────────────────────────────────────────────────────────────
void WorkspaceWidget::saveToSettings(QSettings *s, const QString &prefix) const
{
    s->beginGroup(prefix);
    s->setValue(QStringLiteral("paneCount"), m_paneCount);
    for (int i = 0; i < 4; ++i) {
        if (!m_panes[i]) continue;
        s->beginGroup(QStringLiteral("pane%1").arg(i));
        s->setValue(QStringLiteral("tabs"),       m_panes[i]->tabPaths());
        s->setValue(QStringLiteral("currentTab"), m_panes[i]->currentTabIndex());
        s->endGroup();
    }
    s->endGroup();
}

void WorkspaceWidget::restoreFromSettings(QSettings *s, const QString &prefix)
{
    s->beginGroup(prefix);
    const int count = s->value(QStringLiteral("paneCount"), 1).toInt();
    applyLayout(count);
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

// ─────────────────────────────────────────────────────────────────────────────
// Layout-preset restore
// ─────────────────────────────────────────────────────────────────────────────
void WorkspaceWidget::restoreFromPreset(int count,
                                        const std::array<PaneState, 4> &panes)
{
    applyLayout(count);
    for (int i = 0; i < 4 && i < count; ++i) {
        if (!m_panes[i]) continue;
        if (!panes[i].tabs.isEmpty())
            m_panes[i]->restoreTabs(panes[i].tabs, panes[i].currentTab);
    }
}
