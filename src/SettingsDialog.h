#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QDialogButtonBox>

class SettingsManager;

/**
 * @brief SettingsDialog — multi-tab preferences dialog.
 *
 * Tabs:
 *  1. General   — session restore, language, pane count
 *  2. Appearance — theme, font
 *  3. File Ops  — confirm delete, checksum verify, conflict default
 *  4. Shortcuts — read-only table of keyboard shortcuts
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(SettingsManager *mgr, QWidget *parent = nullptr);

private slots:
    void onAccepted();

private:
    void setupUi();
    QWidget *buildGeneralTab();
    QWidget *buildAppearanceTab();
    QWidget *buildFileOpsTab();
    QWidget *buildShortcutsTab();

    SettingsManager  *m_mgr{nullptr};
    QTabWidget       *m_tabs{nullptr};
    QDialogButtonBox *m_buttons{nullptr};

    // General
    QCheckBox *m_restoreSession{nullptr};
    QComboBox *m_language{nullptr};
    QSpinBox  *m_paneCount{nullptr};

    // Appearance
    QComboBox *m_theme{nullptr};

    // File ops
    QCheckBox *m_confirmDelete{nullptr};
    QCheckBox *m_verifyCopy{nullptr};
    QCheckBox *m_showThumbnails{nullptr};
    QCheckBox *m_showHidden{nullptr};
};
