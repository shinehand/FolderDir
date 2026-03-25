#pragma once

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QJsonArray>

/**
 * @brief BookmarkEntry — a single bookmark item.
 */
struct BookmarkEntry {
    QString name;
    QString path;
    QString iconName;  ///< optional icon name from theme

    bool operator==(const BookmarkEntry &o) const {
        return path == o.path;
    }
};

/**
 * @brief BookmarkManager — manages user bookmarks with persistence.
 *
 * Bookmarks are saved to a JSON file in the application's config directory.
 * Emits bookmarksChanged() whenever the list is mutated.
 */
class BookmarkManager : public QObject
{
    Q_OBJECT

public:
    explicit BookmarkManager(QObject *parent = nullptr);

    const QList<BookmarkEntry> &bookmarks() const { return m_bookmarks; }

    void add(const QString &path, const QString &name = QString());
    void remove(int index);
    void remove(const QString &path);
    void move(int from, int to);
    bool contains(const QString &path) const;
    void setName(int index, const QString &name);
    void setIcon(int index, const QString &iconName);

    bool exportToFile(const QString &filePath) const;
    bool importFromFile(const QString &filePath);

signals:
    void bookmarksChanged();

private:
    void load();
    void save() const;
    QString storageFilePath() const;

    QList<BookmarkEntry> m_bookmarks;
};
