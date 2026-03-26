#pragma once

#include <QtCore/QObject>
#include <QtCore/QSettings>
#include <QtGui/QColor>
#include <QtGui/QPalette>

/**
 * @brief Theme — application colour theme.
 */
enum class Theme {
    System,
    Light,
    Dark
};

/**
 * @brief SettingsManager — centralized settings store.
 *
 * Wraps QSettings to provide typed accessors and a changed() signal so
 * all components can react to settings updates.
 */
class SettingsManager : public QObject
{
    Q_OBJECT

public:
    explicit SettingsManager(QObject *parent = nullptr);

    // General
    int     paneCount() const;
    void    setPaneCount(int n);
    Theme   theme() const;
    void    setTheme(Theme t);
    QString language() const;
    void    setLanguage(const QString &lang);
    bool    restoreSession() const;
    void    setRestoreSession(bool v);

    // File view
    bool    showHiddenFiles() const;
    void    setShowHiddenFiles(bool v);
    bool    showFileExtensions() const;
    void    setShowFileExtensions(bool v);
    bool    confirmDelete() const;
    void    setConfirmDelete(bool v);
    bool    verifyCopyChecksum() const;
    void    setVerifyCopyChecksum(bool v);
    bool    useTrash() const;               ///< GAP-002: 삭제 시 휴지통으로 이동
    void    setUseTrash(bool v);
    bool    showThumbnails() const;
    void    setShowThumbnails(bool v);

    // Session (raw QSettings access for complex data)
    QSettings *raw() { return &m_settings; }

    /** Apply the currently configured theme to the QApplication. */
    void applyTheme() const;

signals:
    void settingsChanged();

private:
    QSettings m_settings;
};
