#include "ColorManager.h"

#include <QtCore/QSettings>
#include <QtCore/QRegularExpression>

ColorManager *ColorManager::s_instance = nullptr;

// ──────────────────────────────────────────────────────────────────────────────
ColorManager *ColorManager::instance()
{
    if (!s_instance)
        s_instance = new ColorManager();
    return s_instance;
}

// ──────────────────────────────────────────────────────────────────────────────
ColorManager::ColorManager(QObject *parent)
    : QObject(parent)
{
    load();
}

// ──────────────────────────────────────────────────────────────────────────────
void ColorManager::setRules(const QVector<ColorRule> &rules)
{
    m_rules = rules;
    save();
    emit rulesChanged();
}

// ──────────────────────────────────────────────────────────────────────────────
bool ColorManager::colorsForFile(const QString &fileName,
                                 QColor &outBg, QColor &outFg) const
{
    for (const ColorRule &rule : m_rules) {
        const QString regexStr =
            QRegularExpression::wildcardToRegularExpression(rule.pattern);
        QRegularExpression re(regexStr,
                              QRegularExpression::CaseInsensitiveOption);
        if (re.match(fileName).hasMatch()) {
            outBg = rule.background;
            outFg = rule.foreground;
            return true;
        }
    }
    return false;
}

// ──────────────────────────────────────────────────────────────────────────────
void ColorManager::load()
{
    QSettings settings(QStringLiteral("FolderDir"), QStringLiteral("FolderDir"));
    const int size = settings.beginReadArray(QStringLiteral("colors/rules"));

    if (size == 0) {
        settings.endArray();
        // Default rules
        m_rules = {
            { QStringLiteral("*.cpp"), QColor(0xAD, 0xD8, 0xE6), QColor() },
            { QStringLiteral("*.h"),   QColor(0x90, 0xEE, 0x90), QColor() },
            { QStringLiteral("*.py"),  QColor(0xFF, 0xFF, 0xE0), QColor() },
            { QStringLiteral("*.md"),  QColor(0xD3, 0xD3, 0xD3), QColor() },
        };
        return;
    }

    m_rules.clear();
    m_rules.reserve(size);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        ColorRule rule;
        rule.pattern = settings.value(QStringLiteral("pattern")).toString();
        const QString bgStr = settings.value(QStringLiteral("background")).toString();
        const QString fgStr = settings.value(QStringLiteral("foreground")).toString();
        rule.background = bgStr.isEmpty() ? QColor() : QColor(bgStr);
        rule.foreground = fgStr.isEmpty() ? QColor() : QColor(fgStr);
        m_rules.append(rule);
    }
    settings.endArray();
}

// ──────────────────────────────────────────────────────────────────────────────
void ColorManager::save() const
{
    QSettings settings(QStringLiteral("FolderDir"), QStringLiteral("FolderDir"));
    settings.beginWriteArray(QStringLiteral("colors/rules"), m_rules.size());
    for (int i = 0; i < m_rules.size(); ++i) {
        settings.setArrayIndex(i);
        const ColorRule &rule = m_rules.at(i);
        settings.setValue(QStringLiteral("pattern"), rule.pattern);
        settings.setValue(QStringLiteral("background"),
                          rule.background.isValid()
                              ? rule.background.name(QColor::HexArgb)
                              : QString());
        settings.setValue(QStringLiteral("foreground"),
                          rule.foreground.isValid()
                              ? rule.foreground.name(QColor::HexArgb)
                              : QString());
    }
    settings.endArray();
}
