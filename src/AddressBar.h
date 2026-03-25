#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtCore/QStringList>

/**
 * @brief AddressBar — shows the current path as an editable line.
 *
 * Displays the current directory path. Clicking focuses the line-edit
 * so the user can type a new path directly and press Enter to navigate.
 * The breadcrumb-style display is approximated by the path text.
 */
class AddressBar : public QWidget
{
    Q_OBJECT

public:
    explicit AddressBar(QWidget *parent = nullptr);

    QString path() const;
    void setPath(const QString &path);

signals:
    /** Emitted when the user commits a new path (Enter key or focus loss). */
    void pathCommitted(const QString &path);

private slots:
    void onReturnPressed();
    void onEditingFinished();

private:
    void setupUi();

    QHBoxLayout *m_layout{nullptr};
    QLineEdit   *m_lineEdit{nullptr};
    QPushButton *m_goButton{nullptr};

    QString m_lastCommitted;
};
