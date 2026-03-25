#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QTabBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedWidget>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

class AddressBar;
class FileSystemBrowser;
class BookmarkManager;

/**
 * @brief FolderPane — one panel in the multi-pane layout.
 *
 * Contains:
 *  - A tab bar (multiple tabs each holding a path/view)
 *  - An address bar (breadcrumb + editable path)
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

    // Session serialisation helpers
    QStringList tabPaths() const;
    int currentTabIndex() const;
    void restoreTabs(const QStringList &paths, int activeIndex);

public slots:
    void navigateTo(const QString &path);
    void newTab(const QString &path = QString());
    void closeTab(int index);
    void closeCurrentTab();
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

private slots:
    void onTabChanged(int index);
    void onTabCloseRequested(int index);
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

    QTabBar        *m_tabBar{nullptr};
    AddressBar     *m_addressBar{nullptr};
    QStackedWidget *m_stack{nullptr};
    QVBoxLayout    *m_layout{nullptr};

    BookmarkManager *m_bookmarkManager{nullptr};
    bool m_active{false};
};
