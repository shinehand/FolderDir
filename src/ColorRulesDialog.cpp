#include "ColorRulesDialog.h"
#include "ColorManager.h"

#include <QtWidgets/QHeaderView>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLineEdit>
#include <QtGui/QColor>
#include <QtGui/QBrush>

// ──────────────────────────────────────────────────────────────────────────────
ColorRulesDialog::ColorRulesDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Colour Rules"));
    setMinimumSize(520, 340);

    auto *mainLayout = new QVBoxLayout(this);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({tr("Pattern"), tr("Background"), tr("Foreground")});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    m_table->setColumnWidth(1, 120);
    m_table->setColumnWidth(2, 120);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(m_table);

    // Row action buttons
    auto *btnLayout = new QHBoxLayout;
    auto *addBtn    = new QPushButton(tr("Add"),       this);
    auto *removeBtn = new QPushButton(tr("Remove"),    this);
    auto *upBtn     = new QPushButton(tr("Move Up"),   this);
    auto *downBtn   = new QPushButton(tr("Move Down"), this);
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(removeBtn);
    btnLayout->addWidget(upBtn);
    btnLayout->addWidget(downBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    connect(addBtn,    &QPushButton::clicked, this, &ColorRulesDialog::onAddRow);
    connect(removeBtn, &QPushButton::clicked, this, &ColorRulesDialog::onRemoveRow);
    connect(upBtn,     &QPushButton::clicked, this, &ColorRulesDialog::onMoveUp);
    connect(downBtn,   &QPushButton::clicked, this, &ColorRulesDialog::onMoveDown);
    connect(m_table,   &QTableWidget::cellDoubleClicked,
            this, &ColorRulesDialog::onCellDoubleClicked);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ColorRulesDialog::onAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    populateTable();
}

// ──────────────────────────────────────────────────────────────────────────────
static void setColorCell(QTableWidgetItem *item, const QColor &color)
{
    if (color.isValid()) {
        item->setData(Qt::UserRole, color);
        item->setBackground(QBrush(color));
        item->setText(color.name(QColor::HexArgb));
    } else {
        item->setData(Qt::UserRole, QVariant());
        item->setBackground(QBrush());
        item->setText(QStringLiteral("(default)"));
    }
}

// ──────────────────────────────────────────────────────────────────────────────
void ColorRulesDialog::populateTable()
{
    const QVector<ColorRule> &rules = ColorManager::instance()->rules();
    m_table->setRowCount(rules.size());
    for (int i = 0; i < rules.size(); ++i) {
        const ColorRule &r = rules.at(i);
        auto *patItem = new QTableWidgetItem(r.pattern);
        auto *bgItem  = new QTableWidgetItem;
        auto *fgItem  = new QTableWidgetItem;
        setColorCell(bgItem, r.background);
        setColorCell(fgItem, r.foreground);
        m_table->setItem(i, 0, patItem);
        m_table->setItem(i, 1, bgItem);
        m_table->setItem(i, 2, fgItem);
    }
}

// ──────────────────────────────────────────────────────────────────────────────
void ColorRulesDialog::onAddRow()
{
    bool ok = false;
    const QString pattern = QInputDialog::getText(
        this, tr("Add Rule"), tr("File name pattern (e.g. *.cpp):"),
        QLineEdit::Normal, QString(), &ok);
    if (!ok || pattern.trimmed().isEmpty()) return;

    const int row = m_table->rowCount();
    m_table->insertRow(row);

    auto *patItem = new QTableWidgetItem(pattern.trimmed());
    auto *bgItem  = new QTableWidgetItem;
    auto *fgItem  = new QTableWidgetItem;
    setColorCell(bgItem, QColor());
    setColorCell(fgItem, QColor());
    m_table->setItem(row, 0, patItem);
    m_table->setItem(row, 1, bgItem);
    m_table->setItem(row, 2, fgItem);
    m_table->selectRow(row);
}

// ──────────────────────────────────────────────────────────────────────────────
void ColorRulesDialog::onRemoveRow()
{
    const int row = m_table->currentRow();
    if (row >= 0) m_table->removeRow(row);
}

// ──────────────────────────────────────────────────────────────────────────────
void ColorRulesDialog::onMoveUp()
{
    const int row = m_table->currentRow();
    if (row <= 0) return;
    for (int col = 0; col < 3; ++col) {
        auto *above = m_table->takeItem(row - 1, col);
        auto *cur   = m_table->takeItem(row,     col);
        m_table->setItem(row - 1, col, cur);
        m_table->setItem(row,     col, above);
    }
    m_table->selectRow(row - 1);
}

// ──────────────────────────────────────────────────────────────────────────────
void ColorRulesDialog::onMoveDown()
{
    const int row = m_table->currentRow();
    if (row < 0 || row >= m_table->rowCount() - 1) return;
    for (int col = 0; col < 3; ++col) {
        auto *below = m_table->takeItem(row + 1, col);
        auto *cur   = m_table->takeItem(row,     col);
        m_table->setItem(row + 1, col, cur);
        m_table->setItem(row,     col, below);
    }
    m_table->selectRow(row + 1);
}

// ──────────────────────────────────────────────────────────────────────────────
void ColorRulesDialog::onCellDoubleClicked(int row, int column)
{
    if (column != 1 && column != 2) return;

    auto *item = m_table->item(row, column);
    if (!item) return;

    const QVariant stored = item->data(Qt::UserRole);
    const QColor initial  = stored.isValid() ? stored.value<QColor>() : QColor();

    QColorDialog dlg(this);
    dlg.setWindowTitle(column == 1 ? tr("Choose Background Colour")
                                   : tr("Choose Foreground Colour"));
    dlg.setOptions(QColorDialog::ShowAlphaChannel);
    if (initial.isValid()) dlg.setCurrentColor(initial);

    // Extra "Clear" button
    auto *clearBtn = new QPushButton(tr("Clear (use default)"), &dlg);
    auto *btnBox   = dlg.findChild<QDialogButtonBox *>();
    if (btnBox) btnBox->addButton(clearBtn, QDialogButtonBox::ResetRole);

    bool cleared = false;
    connect(clearBtn, &QPushButton::clicked, &dlg, [&cleared, &dlg]() {
        cleared = true;
        dlg.reject();
    });

    if (dlg.exec() == QDialog::Accepted) {
        setColorCell(item, dlg.selectedColor());
    } else if (cleared) {
        setColorCell(item, QColor());
    }
}

// ──────────────────────────────────────────────────────────────────────────────
void ColorRulesDialog::onAccepted()
{
    QVector<ColorRule> rules;
    rules.reserve(m_table->rowCount());
    for (int i = 0; i < m_table->rowCount(); ++i) {
        ColorRule rule;
        const auto *patItem = m_table->item(i, 0);
        const auto *bgItem  = m_table->item(i, 1);
        const auto *fgItem  = m_table->item(i, 2);
        rule.pattern = patItem ? patItem->text() : QString();

        if (bgItem) {
            const QVariant v = bgItem->data(Qt::UserRole);
            rule.background = v.isValid() ? v.value<QColor>() : QColor();
        }
        if (fgItem) {
            const QVariant v = fgItem->data(Qt::UserRole);
            rule.foreground = v.isValid() ? v.value<QColor>() : QColor();
        }
        if (!rule.pattern.isEmpty())
            rules.append(rule);
    }
    ColorManager::instance()->setRules(rules);
    accept();
}
