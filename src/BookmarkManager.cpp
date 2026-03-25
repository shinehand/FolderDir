#include "BookmarkManager.h"

#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QFileInfo>

// ──────────────────────────────────────────────────────────────────────────────
BookmarkManager::BookmarkManager(QObject *parent)
    : QObject(parent)
{
    load();
}

// ──────────────────────────────────────────────────────────────────────────────
QString BookmarkManager::storageFilePath() const
{
    const QString dir = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/bookmarks.json");
}

void BookmarkManager::load()
{
    QFile f(storageFilePath());
    if (!f.open(QIODevice::ReadOnly)) return;

    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isArray()) return;

    m_bookmarks.clear();
    for (const QJsonValue &v : doc.array()) {
        const QJsonObject obj = v.toObject();
        BookmarkEntry e;
        e.name     = obj.value(QStringLiteral("name")).toString();
        e.path     = obj.value(QStringLiteral("path")).toString();
        e.iconName = obj.value(QStringLiteral("icon")).toString();
        if (!e.path.isEmpty())
            m_bookmarks.append(e);
    }
}

void BookmarkManager::save() const
{
    QJsonArray arr;
    for (const BookmarkEntry &e : m_bookmarks) {
        QJsonObject obj;
        obj[QStringLiteral("name")] = e.name;
        obj[QStringLiteral("path")] = e.path;
        obj[QStringLiteral("icon")] = e.iconName;
        arr.append(obj);
    }
    QFile f(storageFilePath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(QJsonDocument(arr).toJson());
    }
}

// ──────────────────────────────────────────────────────────────────────────────
void BookmarkManager::add(const QString &path, const QString &name)
{
    if (contains(path)) return;
    BookmarkEntry e;
    e.path = path;
    e.name = name.isEmpty() ? QFileInfo(path).fileName() : name;
    if (e.name.isEmpty()) e.name = path;
    m_bookmarks.append(e);
    save();
    emit bookmarksChanged();
}

void BookmarkManager::remove(int index)
{
    if (index < 0 || index >= m_bookmarks.size()) return;
    m_bookmarks.removeAt(index);
    save();
    emit bookmarksChanged();
}

void BookmarkManager::remove(const QString &path)
{
    for (int i = 0; i < m_bookmarks.size(); ++i) {
        if (m_bookmarks[i].path == path) {
            m_bookmarks.removeAt(i);
            save();
            emit bookmarksChanged();
            return;
        }
    }
}

void BookmarkManager::move(int from, int to)
{
    if (from < 0 || from >= m_bookmarks.size() ||
        to   < 0 || to   >= m_bookmarks.size() ||
        from == to) return;
    m_bookmarks.move(from, to);
    save();
    emit bookmarksChanged();
}

bool BookmarkManager::contains(const QString &path) const
{
    for (const BookmarkEntry &e : m_bookmarks)
        if (e.path == path) return true;
    return false;
}

void BookmarkManager::setName(int index, const QString &name)
{
    if (index < 0 || index >= m_bookmarks.size()) return;
    m_bookmarks[index].name = name;
    save();
    emit bookmarksChanged();
}

void BookmarkManager::setIcon(int index, const QString &iconName)
{
    if (index < 0 || index >= m_bookmarks.size()) return;
    m_bookmarks[index].iconName = iconName;
    save();
    emit bookmarksChanged();
}

// ──────────────────────────────────────────────────────────────────────────────
bool BookmarkManager::exportToFile(const QString &filePath) const
{
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    QJsonArray arr;
    for (const BookmarkEntry &e : m_bookmarks) {
        QJsonObject obj;
        obj[QStringLiteral("name")] = e.name;
        obj[QStringLiteral("path")] = e.path;
        obj[QStringLiteral("icon")] = e.iconName;
        arr.append(obj);
    }
    f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    return true;
}

bool BookmarkManager::importFromFile(const QString &filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) return false;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isArray()) return false;

    for (const QJsonValue &v : doc.array()) {
        const QJsonObject obj = v.toObject();
        const QString path = obj.value(QStringLiteral("path")).toString();
        const QString name = obj.value(QStringLiteral("name")).toString();
        if (!path.isEmpty() && !contains(path))
            add(path, name);
    }
    return true;
}
