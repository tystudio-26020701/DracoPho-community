#pragma once

#include <QString>

namespace markshot::cli {

/**
 * 无人值守录制请求参数（供 CLI / MCP 智能体调用）。
 */
struct CliRecordingRequest {
    QString displayKey;       // 显示器持久化键，如 "screen:DP-1" / "all"（二选一）
    QString geometryText;     // "x,y,width,height"（displayKey 为空时使用）
    QString outputPath;       // 输出文件路径（必需）
    QString format;           // "mp4"（默认） / "gif"
    int fps = 15;
    bool includeAudio = false;
    int durationMs = 0;       // 0 表示不限时，等待 stop-recording
    bool waitForFinish = false; // 完成后才输出最终 JSON（轮询状态）
};

/**
 * 查询当前录制状态并输出 JSON。
 * @return 查询完成返回 0。
 */
int printRecordingStatus();

/**
 * 请求停止当前录制并输出 JSON。
 * @return 已提交停止请求返回 0，没有可停止录制时返回 1。
 */
int stopRecordingFromCommandLine();

/**
 * 启动一次无人值守录制并输出 JSON。
 * @param request 录制请求参数。
 * @return 启动成功返回 0，失败或未运行实例返回 1。
 */
int startRecordingFromCommandLine(const CliRecordingRequest &request);

}  // namespace markshot::cli
