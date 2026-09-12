#pragma once

#include <QCommandLineParser>
#include <QString>

#include <optional>

namespace markshot::cli {

// 屏幕截图（--capture-to 路径）的去向。file 写入 --capture-to 指定路径，
// stage 写入临时暂存目录，inline 以 base64 直接内嵌在 JSON 输出中、不落盘。
enum class ScreenCaptureDestination {
    File,
    Inline,
    Stage,
};

// 屏幕截图暂存去向的默认目录（QDir::tempPath() 下的子目录名）。
QString screenStageDirectoryName();

// 解析无头延时秒数（--delay）。返回 nullopt 表示非法：非整数或超出 0-3600。
std::optional<int> parseHeadlessDelaySeconds(const QString &value);

// 解析屏幕截图去向（--capture-destination 的 screen 语义子集：file、
// inline、stage）。返回 nullopt 表示未知值。
std::optional<ScreenCaptureDestination> parseScreenDestination(const QString &name);

// 判定交互式窗口捕获（--capture-window）与任何无头选项的组合冲突。
// 返回第一条冲突说明；无冲突时返回空字符串。--delay 不参与判定：
// 延时在交互式与无头两条链路中均有合法语义。
QString headlessInteractiveConflict(const QCommandLineParser &parser);

// 判定 --doctor 与任何捕获/录制/窗口/文件参数的组合冲突。--doctor 是纯
// 自检命令，静默吞掉其他选项会制造新的误用面，因此一律显式报错。
// 返回第一条冲突说明；无冲突时返回空字符串。
QString doctorOptionConflict(const QCommandLineParser &parser);

} // namespace markshot::cli
