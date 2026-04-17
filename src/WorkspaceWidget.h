#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QSplitter>
#include <QtCore/QSettings>
#include <array>

#include "FileSystemBrowser.h"  // ViewMode enum
#include "LayoutManager.h"      // PaneState

class FolderPane;
class BookmarkManager;
class SettingsManager;

/**
 * @brief WorkspaceWidget — one workspace tab containing 1–4 FolderPanes.
 *
 * Encapsulates the 2×2 QSplitter grid and the FolderPane array used by a
 * single workspace entry.  MainWindow creates one WorkspaceWidget per
 * workspace tab inside its top-level QTabWidget.
 *
 * Each FolderPane already carries its own BreadcrumbBar (address bar) and
 * its own inner tab bar, so all per-pane navigation features are preserved.
 */
class WorkspaceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WorkspaceWidget(BookmarkManager *bm, SettingsManager *sm,
                             QWidget *parent = nullptr);

    // ── Pane access ──────────────────────────────────────────────────────────
    FolderPane *pane(int index) const;
    int paneCount() const { return m_paneCount; }
    void applyLayout(int count);

    FolderPane *activePane() const { return m_activePane; }
    void setActivePane(FolderPane *p);

    /** Path of the first visible pane that is not the active one (F5/F6 dest). */
    QString otherPanePath() const;

    // ── Pane-sync (SP-9 per-workspace) ───────────────────────────────────────
    bool isSyncEnabled() const { return m_paneSyncEnabled; }
    void setSyncEnabled(bool on) { m_paneSyncEnabled = on; }

    // ── Session persistence ──────────────────────────────────────────────────
    void saveToSettings(QSettings *s, const QString &prefix) const;
    void restoreFromSettings(QSettings *s, const QString &prefix);

    // ── Layout-preset restore ────────────────────────────────────────────────
    void restoreFromPreset(int paneCount, const std::array<PaneState, 4> &panes);

signals:
    /** Forwarded from the child FolderPane that was just activated. */
    void paneActivated(FolderPane *pane);
    /** Forwarded from the active pane when it navigates to a new directory. */
    void pathChanged(const QString &path);
    /** Forwarded from any pane when its selection changes. */
    void selectionChanged();

private slots:
    void onChildPaneActivated(FolderPane *pane);
    void onChildPathChanged(const QString &path, FolderPane *srcPane);

private:
    void createPanes();

    std::array<FolderPane *, 4> m_panes{};
    int         m_paneCount{1};
    FolderPane *m_activePane{nullptr};
    bool        m_paneSyncEnabled{false};

    QSplitter *m_hSplitter{nullptr};
    QSplitter *m_topSplitter{nullptr};
    QSplitter *m_botSplitter{nullptr};

    BookmarkManager *m_bookmarkManager{nullptr};
    SettingsManager *m_settingsManager{nullptr};
};
