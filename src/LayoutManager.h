#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QSettings>
#include <array>
#include <vector>

/**
 * @brief Per-pane state stored in a layout preset.
 */
struct PaneState
{
    QStringList tabs;      ///< Absolute paths of open tabs
    int         currentTab{0};
};

/**
 * @brief A named layout preset.
 *
 * Stores pane count and per-pane navigation state so the user can
 * switch back to a favourite working configuration instantly.
 */
struct LayoutPreset
{
    QString                 name;
    int                     paneCount{4};
    std::array<PaneState, 4> panes{};

    bool isValid() const { return !name.isEmpty() && paneCount >= 1; }
};

/**
 * @brief LayoutManager — save / load up to MaxPresets named layout presets.
 *
 * Sprint 10 (Dev C): provides a persistent registry of layout favourites
 * stored under the "Layouts" group in QSettings.  The host (MainWindow)
 * calls capture() to snapshot the current state, save() to persist it
 * under a user-supplied name, load() to restore a named preset, and
 * presets() to enumerate all saved presets.
 */
class LayoutManager : public QObject
{
    Q_OBJECT

public:
    static constexpr int MaxPresets = 10; ///< Maximum number of saved presets

    explicit LayoutManager(QSettings *settings, QObject *parent = nullptr);

    // ── Persistence ──────────────────────────────────────────────────────────
    void loadAll();   ///< Load all presets from QSettings
    void saveAll();   ///< Persist all presets to QSettings

    // ── Preset management ────────────────────────────────────────────────────
    const std::vector<LayoutPreset> &presets() const { return m_presets; }

    /** Add or overwrite a preset under @p name. Returns true on success. */
    bool save(const LayoutPreset &preset);

    /** Remove the preset with the given @p name. */
    bool remove(const QString &name);

    /** Return a copy of the preset with the given @p name, or invalid preset. */
    LayoutPreset find(const QString &name) const;

    /** Return true if a preset named @p name already exists. */
    bool exists(const QString &name) const;

    /** Return a list of all preset names in insertion order. */
    QStringList names() const;

signals:
    void presetsChanged(); ///< Emitted whenever the preset list is mutated

private:
    QSettings              *m_settings{nullptr};
    std::vector<LayoutPreset> m_presets;
};
