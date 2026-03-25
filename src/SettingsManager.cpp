#include "SettingsManager.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QStyle>
#include <QtCore/QStandardPaths>

// ──────────────────────────────────────────────────────────────────────────────
SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings(QStringLiteral("FolderDir"), QStringLiteral("FolderDir"))
{
}

// ──────────────────────────────────────────────────────────────────────────────
int SettingsManager::paneCount() const
{
    return m_settings.value(QStringLiteral("general/paneCount"), 4).toInt();
}
void SettingsManager::setPaneCount(int n)
{
    m_settings.setValue(QStringLiteral("general/paneCount"), n);
}

Theme SettingsManager::theme() const
{
    const int v = m_settings.value(QStringLiteral("general/theme"), 0).toInt();
    return static_cast<Theme>(v);
}
void SettingsManager::setTheme(Theme t)
{
    m_settings.setValue(QStringLiteral("general/theme"), static_cast<int>(t));
    applyTheme();
    emit settingsChanged();
}

QString SettingsManager::language() const
{
    return m_settings.value(QStringLiteral("general/language"),
                            QStringLiteral("en")).toString();
}
void SettingsManager::setLanguage(const QString &lang)
{
    m_settings.setValue(QStringLiteral("general/language"), lang);
}

bool SettingsManager::restoreSession() const
{
    return m_settings.value(QStringLiteral("general/restoreSession"), true).toBool();
}
void SettingsManager::setRestoreSession(bool v)
{
    m_settings.setValue(QStringLiteral("general/restoreSession"), v);
}

bool SettingsManager::showHiddenFiles() const
{
    return m_settings.value(QStringLiteral("view/showHidden"), false).toBool();
}
void SettingsManager::setShowHiddenFiles(bool v)
{
    m_settings.setValue(QStringLiteral("view/showHidden"), v);
}

bool SettingsManager::showFileExtensions() const
{
    return m_settings.value(QStringLiteral("view/showExtensions"), true).toBool();
}
void SettingsManager::setShowFileExtensions(bool v)
{
    m_settings.setValue(QStringLiteral("view/showExtensions"), v);
}

bool SettingsManager::confirmDelete() const
{
    return m_settings.value(QStringLiteral("fileops/confirmDelete"), true).toBool();
}
void SettingsManager::setConfirmDelete(bool v)
{
    m_settings.setValue(QStringLiteral("fileops/confirmDelete"), v);
}

bool SettingsManager::verifyCopyChecksum() const
{
    return m_settings.value(QStringLiteral("fileops/verifyChecksum"), false).toBool();
}
void SettingsManager::setVerifyCopyChecksum(bool v)
{
    m_settings.setValue(QStringLiteral("fileops/verifyChecksum"), v);
}

bool SettingsManager::showThumbnails() const
{
    return m_settings.value(QStringLiteral("view/thumbnails"), true).toBool();
}
void SettingsManager::setShowThumbnails(bool v)
{
    m_settings.setValue(QStringLiteral("view/thumbnails"), v);
}

// ──────────────────────────────────────────────────────────────────────────────
void SettingsManager::applyTheme() const
{
    switch (theme()) {
    case Theme::Dark: {
        // Simple dark palette
        QPalette p;
        p.setColor(QPalette::Window,          QColor(45,  45,  45));
        p.setColor(QPalette::WindowText,      QColor(220, 220, 220));
        p.setColor(QPalette::Base,            QColor(30,  30,  30));
        p.setColor(QPalette::AlternateBase,   QColor(45,  45,  45));
        p.setColor(QPalette::Text,            QColor(220, 220, 220));
        p.setColor(QPalette::Button,          QColor(60,  60,  60));
        p.setColor(QPalette::ButtonText,      QColor(220, 220, 220));
        p.setColor(QPalette::Highlight,       QColor(42, 130, 218));
        p.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
        p.setColor(QPalette::ToolTipBase,     QColor(50,  50,  50));
        p.setColor(QPalette::ToolTipText,     QColor(220, 220, 220));
        QApplication::setPalette(p);
        break;
    }
    case Theme::Light: {
        QApplication::setPalette(QApplication::style()->standardPalette());
        break;
    }
    case Theme::System:
    default:
        // Restore default
        QApplication::setPalette(QPalette());
        break;
    }
}
