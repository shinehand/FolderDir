#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QTabBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QSet>

#include "FileSystemBrowser.h"  // for ViewMode enum

class BreadcrumbBar;
class BookmarkManager;
class DraggableTabBar;

/**
 * @brief FolderPane — one panel in the multi-pane layout.
 *
 * Contains:
 *  - A tab bar (multiple tabs each holding a path/view)
 *  - A BreadcrumbBar (clickable path segments + editable fallback)
 *  - A view-mode selector button (Details / List / Icons / Thumbnails) — UX-B01
 *  - A FileSystemBrowser (the actual file list view)
 */
class FolderPane : public QWidget
{
    Q_OBJECT

public:
    explicit FolderPane(BookmarkManager *bm, QWidget *parent = nullptr);
    ~FolderPane() override;

    /** Returns the path shown in the currently active tab. */
    QString currentPath() const;

    /** Returns the currently active browser widget. */
    FileSystemBrowser *currentBrowser() const;

    /** Returns true when this pane has keyboard/mouse focus. */
    bool isActive() const { return m_active; }
    void setActive(bool active);

    /** Get / set the view mode for this pane (UX-B01). */
    ViewMode viewMode() const { return m_viewMode; }
    void setViewMode(ViewMode mode);

    // Session serialisation helpers
    QStringList tabPaths() const;
    int currentTabIndex() const;
    void restoreTabs(const QStringList &paths, int activeIndex);

public slots:
    void navigateTo(const QString &path);
    void newTab(const QString &path = QString());
    void closeTab(int index);
    void closeCurrentTab();
    void nextTab();
    void navigateBack();
    void navigateForward();
    void navigateUp();
    void refresh();

signals:
    /** Emitted when the user activates this pane (click / focus). */
    void paneActivated(FolderPane *pane);
    /** Emitted whenever the current directory changes. */
    void pathChanged(const QString &path);
    /** Emitted when selection changes (for status bar). */
    void selectionChanged();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onTabChanged(int index);
    void onTabCloseRequested(int index);
    void onTabDoubleClicked(int index);
    void onAddressBarCommit(const QString &path);
    void onBrowserPathChanged(const QString &path);
    void onBrowserSelectionChanged();
    void onNewTabRequested();
    void onTabContextMenu(const QPoint &pos);

private:
    void setupUi();
    FileSystemBrowser *browserAt(int index) const;
    void addTabInternal(const QString &path);
    void syncAddressBar();
    void setActiveStyle();
    /** Show close buttons only when there are ≥2 tabs (matches browser convention). */
    void updateTabCloseButtons();
    /** Update the view-mode button icon/text to match @p mode. */
    void syncViewModeButton(ViewMode mode);

    DraggableTabBar *m_tabBar{nullptr};
    BreadcrumbBar   *m_addressBar{nullptr};
    QStackedWidget  *m_stack{nullptr};
    QVBoxLayout     *m_layout{nullptr};
    QToolButton     *m_viewModeBtn{nullptr};  ///< Per-pane view-mode selector (UX-B01)

    BookmarkManager *m_bookmarkManager{nullptr};
    bool     m_active{false};
    ViewMode m_viewMode{ViewMode::Details};   ///< Per-pane view mode (UX-B01)

    /** When true the pane is acting as a file-drop target; renders highlight border. */
    bool m_dropHighlight{false};

    /** Registry of all live FolderPane instances for drag-drop pointer validation. */
    static QSet<FolderPane *> s_liveInstances;
};
