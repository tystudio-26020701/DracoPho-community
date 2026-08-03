#include "cli/recording_cli.h"

#include "ipc/single_instance_ipc.h"
#include "recording/recording_file_naming.h"

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QTextStream>

namespace markshot::cli {
namespace {

/**
 * 输出单实例响应 JSON。
 * @param response 单实例响应。
 * @return 无返回值。
 */
void writeResponseJson(const markshot::ipc::SingleInstanceResponse &response)
{
    QTextStream stream(stdout);
    stream << QString::fromUtf8(QJsonDocument(markshot::ipc::responseToJsonObject(response))
                                    .toJson(QJsonDocument::Compact))
           << Qt::endl;
}

/**
 * 创建无运行实例时的录制状态响应。
 * @param message 响应消息。
 * @return 单实例响应。
 */
markshot::ipc::SingleInstanceResponse inactiveResponse(const QString &message)
{
    markshot::ipc::SingleInstanceResponse response;
    response.handled = true;
    response.message = message;
    return response;
}

/**
 * 发送录制状态查询命令。
 * @param status 输出录制状态。
 * @return 查询成功返回 true。
 */
bool fetchRecordingStatus(markshot::recording::RecordingStatus *status)
{
    markshot::ipc::SingleInstanceCommand command;
    command.recordingStatus = true;

    markshot::ipc::SingleInstanceResponse response;
    QString error;
    if (!markshot::ipc::sendSingleInstanceCommand(command, &response, &error)) {
        return false;
    }
    if (status) {
        *status = response.recording;
    }
    return true;
}

/**
 * 轮询等待录制结束（含输出文件最终化）。
 * @param timeoutMs 最大等待时长。
 * @param response 输出最终响应。
 * @return 录制已结束（成功或失败）返回 true，超时或实例消失返回 false。
 */
bool waitForRecordingFinish(int timeoutMs,
                            markshot::ipc::SingleInstanceResponse *response)
{
    const int deadline = timeoutMs > 0 ? timeoutMs : 15 * 60 * 1000;
    markshot::recording::RecordingStatus lastStatus;
    bool inactiveSeen = false;
    qint64 stableSize = -1;
    // 轮询间隔 250ms，最大等待由 deadline 控制。
    for (int waited = 0; waited < deadline; waited += 250) {
        if (fetchRecordingStatus(&lastStatus)) {
            if (lastStatus.active) {
                inactiveSeen = false;
                stableSize = -1;
            } else if (!inactiveSeen) {
                // active=false 只表示停止已请求；异步 writer + remux 还在
                // 最终化。记录起点并等待输出文件出现且大小稳定。
                inactiveSeen = true;
                stableSize = -1;
            }
        } else {
            if (response) {
                *response = inactiveResponse(QStringLiteral("no running instance"));
            }
            return false;
        }

        // 失败快速返回：录制结束且标记失败（例如零帧/编码失败），
        // 立即用真实错误结束，不再等到超时。
        if (inactiveSeen && lastStatus.failed) {
            if (response) {
                response->recording = lastStatus;
            }
            return true;
        }
        // 成功返回：输出文件出现且大小稳定（remux 已完成）。
        if (inactiveSeen) {
            const QFileInfo info(lastStatus.outputPath);
            const qint64 size = info.exists() ? info.size() : -1;
            if (size > 0 && size == stableSize) {
                if (response) {
                    response->recording = lastStatus;
                }
                return true;
            }
            stableSize = size;
        }
        QThread::msleep(250);
    }
    if (response) {
        response->recording = lastStatus;
    }
    return false;
}

}  // namespace

int printRecordingStatus()
{
    markshot::ipc::SingleInstanceCommand command;
    command.recordingStatus = true;

    markshot::ipc::SingleInstanceResponse response;
    QString error;
    if (!markshot::ipc::sendSingleInstanceCommand(command, &response, &error)) {
        writeResponseJson(inactiveResponse(QStringLiteral("no running instance")));
        return 0;
    }

    writeResponseJson(response);
    return 0;
}

int stopRecordingFromCommandLine()
{
    markshot::ipc::SingleInstanceCommand command;
    command.stopRecording = true;

    markshot::ipc::SingleInstanceResponse response;
    QString error;
    if (!markshot::ipc::sendSingleInstanceCommand(command, &response, &error)) {
        writeResponseJson(inactiveResponse(QStringLiteral("no active recording")));
        return 1;
    }

    writeResponseJson(response);
    return response.stopped ? 0 : 1;
}

int startRecordingFromCommandLine(const CliRecordingRequest &request)
{
    markshot::ipc::SingleInstanceCommand command;
    command.startRecording = true;
    command.recordDisplayKey = request.displayKey;
    command.recordGeometryText = request.geometryText;
    command.recordOutputPath = request.outputPath;
    command.recordFormat = request.format.isEmpty() ? QStringLiteral("mp4") : request.format;
    command.recordFps = request.fps;
    command.recordIncludeAudio = request.includeAudio;
    command.recordDurationMs = request.durationMs;

    markshot::ipc::SingleInstanceResponse response;
    QString error;
    if (!markshot::ipc::sendSingleInstanceCommand(command, &response, &error)) {
        writeResponseJson(inactiveResponse(QStringLiteral("no running instance")));
        return 1;
    }
    if (!response.recordingStarted) {
        writeResponseJson(response);
        return 1;
    }

    if (request.waitForFinish) {
        const int timeoutMs = request.durationMs > 0 ? request.durationMs + 60 * 1000 : 15 * 60 * 1000;
        const bool finished = waitForRecordingFinish(timeoutMs, &response);
        if (!finished) {
            // 超时或实例消失：明确告知，绝不能伪装成成功。
            response.message = QStringLiteral("recording did not finish in time");
            writeResponseJson(response);
            return 2;
        }
        // 失败：真实原因由 recordingStatus.failed/errorMessage 提供，快速返回。
        if (response.recording.failed) {
            response.message = response.recording.errorMessage.isEmpty()
                ? QStringLiteral("recording failed")
                : response.recording.errorMessage;
            writeResponseJson(response);
            return 2;
        }
        // 成功：输出文件必须已落盘且非空（remux 完成）。
        const QFileInfo outputInfo(response.recording.outputPath);
        if (!outputInfo.exists() || outputInfo.size() <= 0) {
            response.message = QStringLiteral("recording finished but no output file was produced");
            writeResponseJson(response);
            return 2;
        }
        response.message = QStringLiteral("recording finished");
    }
    writeResponseJson(response);
    return 0;
}

}  // namespace markshot::cli
