#pragma once

#include <QImage>
#include <QJsonObject>
#include <QString>
#include <QVector>

#include <optional>

class QImage;

namespace markshot::capture_history {

/// @brief 单条截图历史记录。
struct HistoryEntry {
    QString id;           // 稳定 id（时间戳 + 随机后缀）
    QString path;         // PNG 文件绝对路径
    QString name;         // 截图名称（显示器名/文件名）
    qint64 timestampMs = 0; // 捕获时间（自 epoch 起毫秒）
};

/// @brief 从配置根对象读取历史开关。
/// @param root 应用配置根对象。
/// @return 启用时返回 true（缺省视为启用）。
bool historyEnabledFromRoot(const QJsonObject &root);

/// @brief 从配置根对象读取历史条目上限。
/// @param root 应用配置根对象。
/// @return 上限值（缺省 50；0 表示无上限）。
int historyMaxEntriesFromRoot(const QJsonObject &root);

/// @brief 截图历史是否启用（history.enabled）。
/// @return 启用时返回 true。
bool enabled();

/// @brief 历史条目数量上限（history.maxEntries，0 表示无上限）。
/// @return 上限值。
int maxEntries();

/// @brief 记录一次截图（复制/保存/贴图时调用）。
/// @param image 截图图像。
/// @param name 截图名称（可为空）。
/// @return 新条目 id；历史被禁用或写入失败时返回空串。
QString recordCapture(const QImage &image, const QString &name);

/// @brief 列出截图历史（按时间倒序，仅含文件仍存在的条目）。
/// @return 历史条目列表。
QVector<HistoryEntry> listCaptures();

/// @brief 删除一条历史记录（文件与索引）。
/// @param id 条目 id。
/// @return 删除成功返回 true。
bool removeCapture(const QString &id);

/// @brief 清空全部截图历史。
/// @return 无返回值。
void clearCaptures();

/// @brief 供测试设置存储目录（覆盖 QStandardPaths 默认目录）。
/// @param directory 目录绝对路径；空串恢复默认。
void setStorageDirectoryForTesting(const QString &directory);

/// @brief 供测试覆盖历史开关与上限（不写配置）。
/// @param enabledOverride 开关覆盖；空 optional 恢复配置读取。
/// @param maxEntriesOverride 上限覆盖；空 optional 恢复配置读取。
void setOverridesForTesting(const std::optional<bool> &enabledOverride,
                            const std::optional<int> &maxEntriesOverride);

}  // namespace markshot::capture_history
