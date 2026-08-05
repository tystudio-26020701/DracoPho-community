#include "history/capture_history.h"

#include "app_config_store.h"
#include "debug_log.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageWriter>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>

#include <algorithm>
#include <optional>

namespace markshot::capture_history {

namespace {

/// @brief 历史索引文件名。
constexpr const char *kIndexFileName = "history.json";

QString g_testDirectory;
std::optional<bool> g_enabledOverride;
std::optional<int> g_maxEntriesOverride;

/// @brief 返回历史存储目录（测试覆盖或默认 AppDataLocation/history）。
QString storageRoot()
{
    if (!g_testDirectory.isEmpty()) {
        return g_testDirectory;
    }
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty()) {
        return {};
    }
    return QDir(base).filePath(QStringLiteral("history"));
}

/// @brief 读取索引 JSON（不存在时返回空对象）。
/// @param directory 历史目录。
/// @return 索引根对象。
QJsonObject readIndex(const QString &directory)
{
    if (directory.isEmpty()) {
        return {};
    }
    const QString indexPath = QDir(directory).filePath(QLatin1String(kIndexFileName));
    QFile file(indexPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return {};
    }
    return document.object();
}

/// @brief 原子写索引 JSON。
/// @param directory 历史目录。
/// @param root 索引根对象。
/// @return 写入成功返回 true。
bool writeIndex(const QString &directory, const QJsonObject &root)
{
    if (directory.isEmpty()) {
        return false;
    }
    QDir().mkpath(directory);
    const QString indexPath = QDir(directory).filePath(QLatin1String(kIndexFileName));
    QSaveFile file(indexPath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return file.commit();
}

/// @brief 生成唯一条目 id。
/// @return id 字符串。
QString makeEntryId()
{
    return QStringLiteral("%1-%2")
        .arg(QDateTime::currentMSecsSinceEpoch())
        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces).left(8));
}

/// @brief 把条目写入索引数组（id -> object）。
/// @param entry 条目。
/// @return JSON 对象。
QJsonObject entryToJson(const HistoryEntry &entry)
{
    QJsonObject object;
    object.insert(QStringLiteral("id"), entry.id);
    object.insert(QStringLiteral("path"), entry.path);
    object.insert(QStringLiteral("name"), entry.name);
    object.insert(QStringLiteral("timestampMs"), entry.timestampMs);
    return object;
}

HistoryEntry entryFromJson(const QJsonObject &object)
{
    HistoryEntry entry;
    entry.id = object.value(QStringLiteral("id")).toString();
    entry.path = object.value(QStringLiteral("path")).toString();
    entry.name = object.value(QStringLiteral("name")).toString();
    entry.timestampMs = object.value(QStringLiteral("timestampMs")).toInteger();
    return entry;
}

}  // namespace

bool historyEnabledFromRoot(const QJsonObject &root)
{
    const QJsonValue enabledValue = root.value(QStringLiteral("history"))
        .toObject()
        .value(QStringLiteral("enabled"));
    if (enabledValue.isBool()) {
        return enabledValue.toBool();
    }
    return true;
}

int historyMaxEntriesFromRoot(const QJsonObject &root)
{
    const QJsonValue maxValue = root.value(QStringLiteral("history"))
        .toObject()
        .value(QStringLiteral("maxEntries"));
    if (maxValue.isDouble()) {
        return std::max(0, maxValue.toInt());
    }
    return 50;
}

bool enabled()
{
    if (g_enabledOverride.has_value()) {
        return *g_enabledOverride;
    }
    bool ok = false;
    const QJsonObject root = readAppConfigRoot(&ok);
    if (!ok) {
        return true;
    }
    return historyEnabledFromRoot(root);
}

int maxEntries()
{
    if (g_maxEntriesOverride.has_value()) {
        return std::max(0, *g_maxEntriesOverride);
    }
    bool ok = false;
    const QJsonObject root = readAppConfigRoot(&ok);
    if (!ok) {
        return 50;
    }
    return historyMaxEntriesFromRoot(root);
}

QString recordCapture(const QImage &image, const QString &name)
{
    if (!enabled() || image.isNull()) {
        return {};
    }

    const QString directory = storageRoot();
    if (directory.isEmpty()) {
        return {};
    }
    QDir().mkpath(directory);

    const QString id = makeEntryId();
    const QString path = QDir(directory).filePath(QStringLiteral("%1.png").arg(id));

    QImageWriter writer(path, QByteArrayLiteral("png"));
    if (!writer.write(image)) {
        markshot::debugLog("history", "failed to write history image %s: %s",
                           path.toUtf8().constData(), writer.errorString().toUtf8().constData());
        return {};
    }
    // 截图可能包含敏感内容：收紧为仅属主可读写，避免被同机其他用户读取。
    QFile::setPermissions(path, QFile::ReadOwner | QFile::WriteOwner);

    HistoryEntry entry;
    entry.id = id;
    entry.path = path;
    entry.name = name;
    entry.timestampMs = QDateTime::currentMSecsSinceEpoch();

    QJsonObject root = readIndex(directory);
    QJsonArray entries = root.value(QStringLiteral("entries")).toArray();
    entries.append(entryToJson(entry));

    // 超过上限时淘汰最旧条目（含文件）。
    const int cap = maxEntries();
    while (cap > 0 && entries.size() > cap) {
        const QJsonObject oldest = entries.first().toObject();
        const QString oldestPath = oldest.value(QStringLiteral("path")).toString();
        if (!oldestPath.isEmpty()) {
            QFile::remove(oldestPath);
        }
        entries.removeAt(0);
    }

    root.insert(QStringLiteral("entries"), entries);
    if (!writeIndex(directory, root)) {
        QFile::remove(path);
        return {};
    }
    return id;
}

QVector<HistoryEntry> listCaptures()
{
    const QJsonObject root = readIndex(storageRoot());
    const QJsonArray entries = root.value(QStringLiteral("entries")).toArray();

    QVector<HistoryEntry> result;
    result.reserve(entries.size());
    for (const QJsonValue &value : entries) {
        if (!value.isObject()) {
            continue;
        }
        const HistoryEntry entry = entryFromJson(value.toObject());
        if (entry.id.isEmpty() || entry.path.isEmpty()) {
            continue;
        }
        // 文件已不存在（被用户手动删除等）的条目自动忽略。
        if (!QFileInfo::exists(entry.path)) {
            continue;
        }
        result.append(entry);
    }
    std::sort(result.begin(), result.end(), [](const HistoryEntry &left, const HistoryEntry &right) {
        return left.timestampMs > right.timestampMs;
    });
    return result;
}

bool removeCapture(const QString &id)
{
    if (id.isEmpty()) {
        return false;
    }
    const QString directory = storageRoot();
    QJsonObject root = readIndex(directory);
    QJsonArray entries = root.value(QStringLiteral("entries")).toArray();

    bool removed = false;
    QJsonArray remaining;
    for (const QJsonValue &value : entries) {
        if (!value.isObject()) {
            continue;
        }
        const HistoryEntry entry = entryFromJson(value.toObject());
        if (entry.id == id) {
            if (!entry.path.isEmpty()) {
                QFile::remove(entry.path);
            }
            removed = true;
            continue;
        }
        remaining.append(value);
    }
    if (!removed) {
        return false;
    }
    root.insert(QStringLiteral("entries"), remaining);
    return writeIndex(directory, root);
}

void clearCaptures()
{
    const QString directory = storageRoot();
    const QJsonObject root = readIndex(directory);
    const QJsonArray entries = root.value(QStringLiteral("entries")).toArray();
    for (const QJsonValue &value : entries) {
        if (value.isObject()) {
            const QString path = value.toObject().value(QStringLiteral("path")).toString();
            if (!path.isEmpty()) {
                QFile::remove(path);
            }
        }
    }
    QJsonObject cleared;
    cleared.insert(QStringLiteral("entries"), QJsonArray());
    writeIndex(directory, cleared);
}

void setStorageDirectoryForTesting(const QString &directory)
{
    g_testDirectory = directory;
}

void setOverridesForTesting(const std::optional<bool> &enabledOverride,
                            const std::optional<int> &maxEntriesOverride)
{
    g_enabledOverride = enabledOverride;
    g_maxEntriesOverride = maxEntriesOverride;
}

}  // namespace markshot::capture_history
