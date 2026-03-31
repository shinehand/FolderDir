#include "LayoutManager.h"

#include <QtCore/QStringList>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

LayoutManager::LayoutManager(QSettings *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
{
    loadAll();
}

// ─────────────────────────────────────────────────────────────────────────────
// Persistence
// ─────────────────────────────────────────────────────────────────────────────

void LayoutManager::loadAll()
{
    m_presets.clear();

    m_settings->beginGroup(QStringLiteral("Layouts"));
    const int count = m_settings->value(QStringLiteral("count"), 0).toInt();

    for (int i = 0; i < count && i < MaxPresets; ++i) {
        m_settings->beginGroup(QStringLiteral("preset%1").arg(i));

        LayoutPreset p;
        p.name      = m_settings->value(QStringLiteral("name")).toString();
        p.paneCount = m_settings->value(QStringLiteral("paneCount"), 4).toInt();

        for (int j = 0; j < 4; ++j) {
            m_settings->beginGroup(QStringLiteral("pane%1").arg(j));
            p.panes[j].tabs       = m_settings->value(QStringLiteral("tabs")).toStringList();
            p.panes[j].currentTab = m_settings->value(QStringLiteral("currentTab"), 0).toInt();
            m_settings->endGroup();
        }

        if (p.isValid())
            m_presets.push_back(std::move(p));

        m_settings->endGroup(); // preset%i
    }

    m_settings->endGroup(); // Layouts
}

void LayoutManager::saveAll()
{
    m_settings->beginGroup(QStringLiteral("Layouts"));
    m_settings->remove(QString()); // clear the group

    m_settings->setValue(QStringLiteral("count"),
                         static_cast<int>(m_presets.size()));

    for (int i = 0; i < static_cast<int>(m_presets.size()); ++i) {
        const LayoutPreset &p = m_presets[i];
        m_settings->beginGroup(QStringLiteral("preset%1").arg(i));

        m_settings->setValue(QStringLiteral("name"),      p.name);
        m_settings->setValue(QStringLiteral("paneCount"), p.paneCount);

        for (int j = 0; j < 4; ++j) {
            m_settings->beginGroup(QStringLiteral("pane%1").arg(j));
            m_settings->setValue(QStringLiteral("tabs"),       p.panes[j].tabs);
            m_settings->setValue(QStringLiteral("currentTab"), p.panes[j].currentTab);
            m_settings->endGroup();
        }

        m_settings->endGroup(); // preset%i
    }

    m_settings->endGroup(); // Layouts
    m_settings->sync();
}

// ─────────────────────────────────────────────────────────────────────────────
// Preset management
// ─────────────────────────────────────────────────────────────────────────────

bool LayoutManager::save(const LayoutPreset &preset)
{
    if (!preset.isValid()) return false;
    if (static_cast<int>(m_presets.size()) >= MaxPresets && !exists(preset.name))
        return false; // capacity reached

    auto it = std::find_if(m_presets.begin(), m_presets.end(),
                           [&](const LayoutPreset &p) { return p.name == preset.name; });
    if (it != m_presets.end())
        *it = preset; // overwrite
    else
        m_presets.push_back(preset);

    saveAll();
    emit presetsChanged();
    return true;
}

bool LayoutManager::remove(const QString &name)
{
    auto it = std::find_if(m_presets.begin(), m_presets.end(),
                           [&](const LayoutPreset &p) { return p.name == name; });
    if (it == m_presets.end()) return false;

    m_presets.erase(it);
    saveAll();
    emit presetsChanged();
    return true;
}

LayoutPreset LayoutManager::find(const QString &name) const
{
    for (const LayoutPreset &p : m_presets)
        if (p.name == name) return p;
    return {}; // invalid preset (empty name)
}

bool LayoutManager::exists(const QString &name) const
{
    return std::any_of(m_presets.begin(), m_presets.end(),
                       [&](const LayoutPreset &p) { return p.name == name; });
}

QStringList LayoutManager::names() const
{
    QStringList result;
    result.reserve(static_cast<int>(m_presets.size()));
    for (const LayoutPreset &p : m_presets)
        result.append(p.name);
    return result;
}
