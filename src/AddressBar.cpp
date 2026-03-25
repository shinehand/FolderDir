#include "AddressBar.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtCore/QDir>

// ──────────────────────────────────────────────────────────────────────────────
AddressBar::AddressBar(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void AddressBar::setupUi()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(2, 2, 2, 2);
    m_layout->setSpacing(2);

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setPlaceholderText(tr("Enter path…"));
    m_lineEdit->setClearButtonEnabled(true);
    m_layout->addWidget(m_lineEdit, 1);

    m_goButton = new QPushButton(tr("Go"), this);
    m_goButton->setFixedWidth(36);
    m_goButton->setToolTip(tr("Navigate to path (Enter)"));
    m_layout->addWidget(m_goButton);

    connect(m_lineEdit, &QLineEdit::returnPressed,
            this, &AddressBar::onReturnPressed);
    connect(m_lineEdit, &QLineEdit::editingFinished,
            this, &AddressBar::onEditingFinished);
    connect(m_goButton, &QPushButton::clicked,
            this, &AddressBar::onReturnPressed);
}

// ──────────────────────────────────────────────────────────────────────────────
QString AddressBar::path() const
{
    return m_lineEdit->text();
}

void AddressBar::setPath(const QString &path)
{
    if (m_lineEdit->hasFocus()) return; // don't overwrite while the user is typing
    m_lineEdit->setText(path);
    m_lastCommitted = path;
}

// ──────────────────────────────────────────────────────────────────────────────
void AddressBar::onReturnPressed()
{
    const QString text = m_lineEdit->text().trimmed();
    if (text.isEmpty() || text == m_lastCommitted) return;

    // Normalise path
    QString resolved = QDir::cleanPath(text);
    if (!QFileInfo::exists(resolved)) {
        // Highlight error but don't crash
        m_lineEdit->setStyleSheet(QStringLiteral("QLineEdit { border: 1px solid red; }"));
        return;
    }
    m_lineEdit->setStyleSheet(QString());
    m_lastCommitted = resolved;
    emit pathCommitted(resolved);
}

void AddressBar::onEditingFinished()
{
    // Only navigate if the text changed from what is currently displayed
    if (m_lineEdit->text() != m_lastCommitted)
        onReturnPressed();
}
