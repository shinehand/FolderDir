#pragma once

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtGui/QColor>
#include <QtCore/QString>

/**
 * @brief ColorRule — maps a glob pattern to background/foreground colours.
 *
 * The pattern is matched case-insensitively against the file or directory name.
 * Standard file glob wildcards (* and ?) are supported.
 * An invalid QColor means "use default".
 */
struct ColorRule {
    QString pattern;    ///< e.g. "*.cpp", "*.h", "Makefile"
    QColor  background; ///< row background; invalid = default
    QColor  foreground; ///< row text colour; invalid = default
};

/**
 * @brief ColorManager — singleton that manages file colour-coding rules.
 *
 * Rules are evaluated in order; the first matching rule wins.
 * Persists rules to QSettings under the "colors/rules" array key.
 */
class ColorManager : public QObject
{
    Q_OBJECT
public:
    static ColorManager *instance();

    const QVector<ColorRule> &rules() const { return m_rules; }
    void setRules(const QVector<ColorRule> &rules);

    /**
     * Returns the matching background and foreground colours for @p fileName.
     * @return true if any rule matched; false if defaults should be used.
     */
    bool colorsForFile(const QString &fileName,
                       QColor &outBg, QColor &outFg) const;

    void load();
    void save() const;

signals:
    void rulesChanged();

private:
    explicit ColorManager(QObject *parent = nullptr);
    static ColorManager *s_instance;
    QVector<ColorRule> m_rules;
};
