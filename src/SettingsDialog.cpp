#include "SettingsDialog.h"
#include "SettingsManager.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QDialogButtonBox>

// ──────────────────────────────────────────────────────────────────────────────
SettingsDialog::SettingsDialog(SettingsManager *mgr, QWidget *parent)
    : QDialog(parent)
    , m_mgr(mgr)
{
    setupUi();
    setWindowTitle(tr("Settings"));
    resize(520, 420);
}

void SettingsDialog::setupUi()
{
    auto *layout = new QVBoxLayout(this);

    m_tabs = new QTabWidget(this);
    m_tabs->addTab(buildGeneralTab(),    tr("General"));
    m_tabs->addTab(buildAppearanceTab(), tr("Appearance"));
    m_tabs->addTab(buildFileOpsTab(),    tr("File Operations"));
    m_tabs->addTab(buildShortcutsTab(),  tr("Shortcuts"));
    layout->addWidget(m_tabs, 1);

    m_buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccepted);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(m_buttons);
}

// ──────────────────────────────────────────────────────────────────────────────
QWidget *SettingsDialog::buildGeneralTab()
{
    auto *w = new QWidget(this);
    auto *form = new QFormLayout(w);

    m_restoreSession = new QCheckBox(tr("Restore previous session on startup"), w);
    m_restoreSession->setChecked(m_mgr->restoreSession());
    form->addRow(m_restoreSession);

    m_paneCount = new QSpinBox(w);
    m_paneCount->setRange(1, 4);
    m_paneCount->setValue(m_mgr->paneCount());
    form->addRow(tr("Default pane count:"), m_paneCount);

    m_language = new QComboBox(w);
    m_language->addItem(tr("English"),  QStringLiteral("en"));
    m_language->addItem(tr("한국어"), QStringLiteral("ko"));
    const int langIdx = m_language->findData(m_mgr->language());
    if (langIdx >= 0) m_language->setCurrentIndex(langIdx);
    form->addRow(tr("Language (requires restart):"), m_language);

    return w;
}

QWidget *SettingsDialog::buildAppearanceTab()
{
    auto *w = new QWidget(this);
    auto *form = new QFormLayout(w);

    m_theme = new QComboBox(w);
    m_theme->addItem(tr("System Default"), 0);
    m_theme->addItem(tr("Light"),          1);
    m_theme->addItem(tr("Dark"),           2);
    m_theme->setCurrentIndex(static_cast<int>(m_mgr->theme()));
    form->addRow(tr("Theme:"), m_theme);

    m_showThumbnails = new QCheckBox(tr("Show image thumbnails"), w);
    m_showThumbnails->setChecked(m_mgr->showThumbnails());
    form->addRow(m_showThumbnails);

    return w;
}

QWidget *SettingsDialog::buildFileOpsTab()
{
    auto *w = new QWidget(this);
    auto *form = new QFormLayout(w);

    m_confirmDelete = new QCheckBox(tr("Confirm before deleting files"), w);
    m_confirmDelete->setChecked(m_mgr->confirmDelete());
    form->addRow(m_confirmDelete);

    m_verifyCopy = new QCheckBox(tr("Verify copy integrity (CRC check)"), w);
    m_verifyCopy->setChecked(m_mgr->verifyCopyChecksum());
    form->addRow(m_verifyCopy);

    m_useTrash = new QCheckBox(tr("Move deleted files to Trash (instead of permanent delete)"), w);
    m_useTrash->setChecked(m_mgr->useTrash());
    form->addRow(m_useTrash);

    m_showHidden = new QCheckBox(tr("Show hidden files and folders"), w);
    m_showHidden->setChecked(m_mgr->showHiddenFiles());
    form->addRow(m_showHidden);

    return w;
}

QWidget *SettingsDialog::buildShortcutsTab()
{
    auto *w = new QWidget(this);
    auto *layout = new QVBoxLayout(w);

    auto *tree = new QTreeWidget(w);
    tree->setColumnCount(2);
    tree->setHeaderLabels({tr("Action"), tr("Shortcut")});
    tree->header()->setStretchLastSection(false);
    tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    static const struct { const char *action; const char *shortcut; } kShortcuts[] = {
        {"Back",           "Alt+Left"},
        {"Forward",        "Alt+Right"},
        {"Up",             "Alt+Up"},
        {"Rename",         "F2"},
        {"Copy",           "Ctrl+C / F5"},
        {"Cut",            "Ctrl+X"},
        {"Paste",          "Ctrl+V / F6"},
        {"Delete",         "Delete / F8"},
        {"New Folder",     "F7 / Ctrl+Shift+N"},
        {"New File",       "Ctrl+N"},
        {"Select All",     "Ctrl+A"},
        {"Copy Path",      "Ctrl+Shift+C"},
        {"Search",         "Ctrl+F"},
        {"Show Hidden",    "Ctrl+H"},
        {"Preview Panel",  "Ctrl+P"},
        {"New Tab",        "Ctrl+T"},
        {"Close Tab",      "Ctrl+W"},
        {"Bookmark",       "Ctrl+D"},
        {"Settings",       "Ctrl+,"},
        {"Pane 1",         "Ctrl+1"},
        {"Pane 2",         "Ctrl+2"},
        {"Pane 3",         "Ctrl+3"},
        {"Pane 4",         "Ctrl+4"},
    };

    for (const auto &row : kShortcuts) {
        auto *item = new QTreeWidgetItem(tree);
        item->setText(0, tr(row.action));
        item->setText(1, QString::fromLatin1(row.shortcut));
    }
    tree->expandAll();
    layout->addWidget(tree);
    layout->addWidget(new QLabel(tr("Shortcut editing will be available in a future version."), w));

    return w;
}

// ──────────────────────────────────────────────────────────────────────────────
void SettingsDialog::onAccepted()
{
    m_mgr->setRestoreSession(m_restoreSession->isChecked());
    m_mgr->setPaneCount(m_paneCount->value());
    m_mgr->setLanguage(m_language->currentData().toString());
    m_mgr->setTheme(static_cast<Theme>(m_theme->currentIndex()));
    m_mgr->setConfirmDelete(m_confirmDelete->isChecked());
    m_mgr->setVerifyCopyChecksum(m_verifyCopy->isChecked());
    m_mgr->setUseTrash(m_useTrash->isChecked());
    m_mgr->setShowThumbnails(m_showThumbnails->isChecked());
    m_mgr->setShowHiddenFiles(m_showHidden->isChecked());
    accept();
}
