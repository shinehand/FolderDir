#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QPushButton>

/**
 * @brief BreadcrumbBar — editable path bar with clickable breadcrumb segments.
 *
 * Default (breadcrumb) mode: each directory segment in the current path is
 * rendered as a QPushButton separated by "›" labels.  Clicking a segment
 * navigates directly to that directory level.  Double-clicking anywhere on
 * the bar, or clicking the pencil button, switches to text-edit mode.
 *
 * Edit mode: a standard QLineEdit.  Pressing Enter commits the path and
 * returns to breadcrumb mode; pressing Escape cancels and returns without
 * navigation.
 *
 * The public interface is intentionally compatible with the old AddressBar so
 * that FolderPane can swap the two classes without other changes.
 */
class BreadcrumbBar : public QWidget
{
    Q_OBJECT

public:
    explicit BreadcrumbBar(QWidget *parent = nullptr);

    /** Returns the currently displayed path. */
    QString path() const;

    /**
     * Sets the displayed path.  If the bar is in edit mode and the user is
     * actively typing, this call is ignored so as not to overwrite input.
     */
    void setPath(const QString &path);

signals:
    /** Emitted when the user commits a new path (Enter key or segment click). */
    void pathCommitted(const QString &path);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onReturnPressed();
    void onEditButtonClicked();
    void onSegmentClicked(const QString &segPath);

private:
    void setupUi();
    void rebuildBreadcrumbs();
    void switchToEditMode();
    void switchToBreadcrumbMode();

    // Page indices in m_stack
    static constexpr int PageBreadcrumb = 0;
    static constexpr int PageEdit       = 1;

    QStackedWidget *m_stack{nullptr};

    // ── Breadcrumb page ───────────────────────────────────────────────────
    QScrollArea    *m_scrollArea{nullptr};
    QWidget        *m_breadcrumbWidget{nullptr};
    QHBoxLayout    *m_breadcrumbLayout{nullptr};

    // ── Edit page ─────────────────────────────────────────────────────────
    QWidget        *m_editWidget{nullptr};
    QHBoxLayout    *m_editLayout{nullptr};
    QLineEdit      *m_lineEdit{nullptr};
    QPushButton    *m_goButton{nullptr};

    QString         m_currentPath;
    QString         m_lastCommitted;
};
