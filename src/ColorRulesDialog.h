#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>

/**
 * @brief ColorRulesDialog — dialog for managing file colour-coding rules.
 *
 * Displays a table of (Pattern, Background, Foreground) rules.
 * Double-clicking a colour cell opens QColorDialog to change or clear the colour.
 */
class ColorRulesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ColorRulesDialog(QWidget *parent = nullptr);

private slots:
    void onAddRow();
    void onRemoveRow();
    void onMoveUp();
    void onMoveDown();
    void onCellDoubleClicked(int row, int column);
    void onAccepted();

private:
    void populateTable();

    QTableWidget *m_table{nullptr};
};
