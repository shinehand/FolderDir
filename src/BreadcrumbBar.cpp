#include "BreadcrumbBar.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFrame>
#include <QtWidgets/QSizePolicy>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QtCore/QTimer>

// ──────────────────────────────────────────────────────────────────────────────
BreadcrumbBar::BreadcrumbBar(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

// ──────────────────────────────────────────────────────────────────────────────
void BreadcrumbBar::setupUi()
{
    auto *outerLayout = new QHBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_stack = new QStackedWidget(this);
    outerLayout->addWidget(m_stack, 1);

    // ── Breadcrumb page ───────────────────────────────────────────────────
    {
        // The scroll area lets the breadcrumb scroll horizontally when the
        // path is very long without resizing the bar vertically.
        m_scrollArea = new QScrollArea(this);
        m_scrollArea->setWidgetResizable(true);
        m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scrollArea->setFrameShape(QFrame::NoFrame);
        m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        m_breadcrumbWidget = new QWidget(m_scrollArea);
        m_breadcrumbLayout = new QHBoxLayout(m_breadcrumbWidget);
        m_breadcrumbLayout->setContentsMargins(2, 0, 2, 0);
        m_breadcrumbLayout->setSpacing(0);
        m_breadcrumbLayout->addStretch(1);

        m_scrollArea->setWidget(m_breadcrumbWidget);

        // Wrap in a QWidget that also has the pencil/edit button at the right
        auto *bcPage = new QWidget(this);
        auto *bcPageLayout = new QHBoxLayout(bcPage);
        bcPageLayout->setContentsMargins(0, 0, 0, 0);
        bcPageLayout->setSpacing(0);
        bcPageLayout->addWidget(m_scrollArea, 1);

        auto *editBtn = new QPushButton(QStringLiteral("✎"), bcPage);
        editBtn->setFixedSize(24, 24);
        editBtn->setFlat(true);
        editBtn->setToolTip(tr("Edit path (double-click)"));
        bcPageLayout->addWidget(editBtn);
        connect(editBtn, &QPushButton::clicked,
                this, &BreadcrumbBar::onEditButtonClicked);

        m_stack->addWidget(bcPage);   // PageBreadcrumb = 0
    }

    // ── Edit page ─────────────────────────────────────────────────────────
    {
        m_editWidget = new QWidget(this);
        m_editLayout = new QHBoxLayout(m_editWidget);
        m_editLayout->setContentsMargins(2, 0, 2, 0);
        m_editLayout->setSpacing(2);

        m_lineEdit = new QLineEdit(m_editWidget);
        m_lineEdit->setPlaceholderText(tr("Enter path…"));
        m_lineEdit->setClearButtonEnabled(true);
        m_editLayout->addWidget(m_lineEdit, 1);

        m_goButton = new QPushButton(tr("Go"), m_editWidget);
        m_goButton->setFixedWidth(36);
        m_goButton->setToolTip(tr("Navigate (Enter)"));
        m_editLayout->addWidget(m_goButton);

        connect(m_lineEdit, &QLineEdit::returnPressed,
                this, &BreadcrumbBar::onReturnPressed);
        connect(m_goButton, &QPushButton::clicked,
                this, &BreadcrumbBar::onReturnPressed);

        // Install key event filter so we can catch Escape inside QLineEdit
        m_lineEdit->installEventFilter(this);

        m_stack->addWidget(m_editWidget);  // PageEdit = 1
    }

    m_stack->setCurrentIndex(PageBreadcrumb);
}

// ──────────────────────────────────────────────────────────────────────────────
bool BreadcrumbBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_lineEdit && event->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Escape) {
            switchToBreadcrumbMode();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

// ──────────────────────────────────────────────────────────────────────────────
QString BreadcrumbBar::path() const
{
    return m_currentPath;
}

void BreadcrumbBar::setPath(const QString &path)
{
    // Don't overwrite while the user is actively editing
    if (m_stack->currentIndex() == PageEdit && m_lineEdit->hasFocus())
        return;

    if (path == m_currentPath) return;
    m_currentPath    = path;
    m_lastCommitted  = path;

    // Keep edit text in sync for when the user switches back to edit mode
    m_lineEdit->setText(path);

    if (m_stack->currentIndex() == PageBreadcrumb)
        rebuildBreadcrumbs();
}

// ──────────────────────────────────────────────────────────────────────────────
void BreadcrumbBar::rebuildBreadcrumbs()
{
    // Remove all existing children from m_breadcrumbLayout
    QLayoutItem *child;
    while ((child = m_breadcrumbLayout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    if (m_currentPath.isEmpty()) {
        m_breadcrumbLayout->addStretch(1);
        return;
    }

    // Split path into segments.  Handle both Unix ( /a/b/c ) and Windows ( C:\a\b ).
    const QString cleaned = QDir::toNativeSeparators(
                                QDir::cleanPath(m_currentPath));
    QStringList segments;

#ifdef Q_OS_WIN
    // On Windows, split on backslash; first segment is the drive (e.g. "C:")
    const QStringList parts = cleaned.split(QDir::separator(), Qt::SkipEmptyParts);
    for (const QString &p : parts)
        segments << p;
#else
    // On Unix, first segment represents "/" itself
    segments << QStringLiteral("/");
    const QStringList parts = m_currentPath.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (const QString &p : parts)
        segments << p;
#endif

    // Build cumulative paths for each segment
    QStringList cumPaths;
    cumPaths.reserve(segments.size());

#ifdef Q_OS_WIN
    QString cumPath;
    for (const QString &seg : segments) {
        if (cumPath.isEmpty())
            cumPath = seg + QDir::separator();
        else {
            cumPath += seg;
            cumPath += QDir::separator();
        }
        cumPaths << QDir::cleanPath(cumPath);
    }
#else
    QString cumPath;
    for (const QString &seg : segments) {
        if (seg == QLatin1String("/")) {
            cumPath = QStringLiteral("/");
        } else {
            cumPath += QLatin1Char('/');
            cumPath += seg;
        }
        cumPaths << cumPath;
    }
#endif

    // Add a button for each segment with a separator "›" in between
    for (int i = 0; i < segments.size(); ++i) {
        if (i > 0) {
            auto *sep = new QLabel(QStringLiteral(" › "), m_breadcrumbWidget);
            sep->setStyleSheet(QStringLiteral("color: palette(mid);"));
            m_breadcrumbLayout->addWidget(sep);
        }
        const QString segLabel = segments.at(i);
        const QString segPath  = cumPaths.at(i);

        auto *btn = new QPushButton(segLabel, m_breadcrumbWidget);
        btn->setFlat(true);
        btn->setStyleSheet(QStringLiteral(
            "QPushButton { border: none; padding: 2px 4px; }"
            "QPushButton:hover { background: palette(highlight); "
            "                    color: palette(highlighted-text); "
            "                    border-radius: 3px; }"));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        connect(btn, &QPushButton::clicked, this,
                [this, segPath]() { onSegmentClicked(segPath); });

        m_breadcrumbLayout->addWidget(btn);
    }

    m_breadcrumbLayout->addStretch(1);

    // Scroll to the rightmost segment (current directory)
    QTimer::singleShot(0, m_scrollArea, [this]() {
        m_scrollArea->horizontalScrollBar()->setValue(
            m_scrollArea->horizontalScrollBar()->maximum());
    });
}

// ──────────────────────────────────────────────────────────────────────────────
void BreadcrumbBar::switchToEditMode()
{
    m_lineEdit->setText(m_currentPath);
    m_stack->setCurrentIndex(PageEdit);
    m_lineEdit->setFocus();
    m_lineEdit->selectAll();
}

void BreadcrumbBar::switchToBreadcrumbMode()
{
    m_lineEdit->setStyleSheet(QString());
    m_stack->setCurrentIndex(PageBreadcrumb);
    rebuildBreadcrumbs();
}

// ──────────────────────────────────────────────────────────────────────────────
void BreadcrumbBar::onEditButtonClicked()
{
    switchToEditMode();
}

void BreadcrumbBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_stack->currentIndex() == PageBreadcrumb)
        switchToEditMode();
    QWidget::mouseDoubleClickEvent(event);
}

void BreadcrumbBar::onReturnPressed()
{
    const QString text = m_lineEdit->text().trimmed();
    if (text.isEmpty()) {
        switchToBreadcrumbMode();
        return;
    }

    const QString resolved = QDir::cleanPath(text);
    if (!QFileInfo::exists(resolved)) {
        m_lineEdit->setStyleSheet(QStringLiteral("QLineEdit { border: 1px solid red; }"));
        return;
    }

    m_lineEdit->setStyleSheet(QString());
    m_currentPath   = resolved;
    m_lastCommitted = resolved;
    switchToBreadcrumbMode();
    emit pathCommitted(resolved);
}

void BreadcrumbBar::onSegmentClicked(const QString &segPath)
{
    if (segPath == m_currentPath) return;
    m_currentPath   = segPath;
    m_lastCommitted = segPath;
    rebuildBreadcrumbs();
    emit pathCommitted(segPath);
}
