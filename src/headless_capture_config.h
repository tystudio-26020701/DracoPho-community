#pragma once

#include <QJsonObject>

namespace markshot {

/// @brief 无头模式下截图结果的默认去向。
enum class HeadlessCaptureDestination {
    Inline,  // base64 直接写入 JSON 返回，不写文件、不碰剪贴板
    Stage,   // 写入系统临时暂存目录，由调用方自行取走
};

/// @brief 无头模式配置。
struct HeadlessCaptureConfig {
    /// @brief 未显式指定 --capture-destination 时的默认去向。
    HeadlessCaptureDestination defaultDestination = HeadlessCaptureDestination::Inline;
    /// @brief 是否允许无头模式写入系统剪贴板。默认关闭，防止脚本意外篡改剪贴板。
    bool clipboardAllowed = false;
};

/// @brief 返回无头去向的规范配置名称。
/// @param destination 无头去向枚举。
/// @return 可写入配置文件的名称（"inline"/"stage"）。
QString headlessCaptureDestinationName(HeadlessCaptureDestination destination);

/// @brief 从配置根对象读取无头模式配置。
/// @param root 应用配置根对象。
/// @return 无头模式配置，配置缺失或非法时返回默认值。
HeadlessCaptureConfig headlessCaptureConfigFromRoot(const QJsonObject &root);

/// @brief 写入无头模式配置到配置根对象。
/// @param root 需要更新的配置根对象。
/// @param config 无头模式配置。
void writeHeadlessCaptureConfig(QJsonObject *root, const HeadlessCaptureConfig &config);

}  // namespace markshot
