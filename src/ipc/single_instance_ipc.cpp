#include "ipc/single_instance_ipc.h"

#include "debug_log.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QLocalSocket>
#include <QObject>

namespace markshot::ipc {
namespace {

/**
 * 将录制状态转换为 JSON 对象。
 * @param status 录制状态。
 * @return JSON 对象。
 */
QJsonObject recordingStatusToJson(const markshot::recording::RecordingStatus &status)
{
    QJsonObject object;
    object.insert(QStringLiteral("active"), status.active);
    object.insert(QStringLiteral("finishedOk"), status.finishedOk);
    object.insert(QStringLiteral("failed"), status.failed);
    object.insert(QStringLiteral("errorMessage"), status.errorMessage);
    object.insert(QStringLiteral("mode"), markshot::recording::recordingModeName(status.mode));
    object.insert(QStringLiteral("fps"), status.fps);
    object.insert(QStringLiteral("frameCount"), status.frameCount);
    object.insert(QStringLiteral("elapsedMs"), static_cast<double>(status.elapsedMs));
    object.insert(QStringLiteral("outputPath"), status.outputPath);
    return object;
}

/**
 * 从 JSON 对象读取录制状态。
 * @param object JSON 对象。
 * @return 录制状态。
 */
markshot::recording::RecordingStatus recordingStatusFromJson(const QJsonObject &object)
{
    markshot::recording::RecordingStatus status;
    status.active = object.value(QStringLiteral("active")).toBool(false);
    status.finishedOk = object.value(QStringLiteral("finishedOk")).toBool(false);
    status.failed = object.value(QStringLiteral("failed")).toBool(false);
    status.errorMessage = object.value(QStringLiteral("errorMessage")).toString();
    const QString modeText = object.value(QStringLiteral("mode")).toString();
    if (modeText == QStringLiteral("video")) {
        status.mode = markshot::recording::RecordingMode::Video;
    } else if (modeText == QStringLiteral("webp")) {
        status.mode = markshot::recording::RecordingMode::Webp;
    } else {
        status.mode = markshot::recording::RecordingMode::Gif;
    }
    status.fps = object.value(QStringLiteral("fps")).toInt(0);
    status.frameCount = object.value(QStringLiteral("frameCount")).toInt(0);
    status.elapsedMs = static_cast<qint64>(object.value(QStringLiteral("elapsedMs")).toDouble(0.0));
    status.outputPath = object.value(QStringLiteral("outputPath")).toString();
    return status;
}

/**
 * 编码单实例命令。
 * @param command 命令内容。
 * @return JSON 行。
 */
QByteArray encodeCommand(const SingleInstanceCommand &command)
{
    QJsonObject object;
    object.insert(QStringLiteral("capture"), command.capture);
    object.insert(QStringLiteral("fullscreen"), command.fullscreen);
    object.insert(QStringLiteral("allOutputs"), command.allOutputs);
    object.insert(QStringLiteral("recordingStatus"), command.recordingStatus);
    object.insert(QStringLiteral("stopRecording"), command.stopRecording);
    object.insert(QStringLiteral("startRecording"), command.startRecording);
    object.insert(QStringLiteral("recordDisplayKey"), command.recordDisplayKey);
    object.insert(QStringLiteral("recordGeometryText"), command.recordGeometryText);
    object.insert(QStringLiteral("recordOutputPath"), command.recordOutputPath);
    object.insert(QStringLiteral("recordFormat"), command.recordFormat);
    object.insert(QStringLiteral("recordFps"), command.recordFps);
    object.insert(QStringLiteral("recordIncludeAudio"), command.recordIncludeAudio);
    object.insert(QStringLiteral("recordDurationMs"), command.recordDurationMs);
    return QJsonDocument(object).toJson(QJsonDocument::Compact) + '\n';
}

/**
 * 解码单实例命令。
 * @param payload JSON 载荷。
 * @return 命令内容。
 */
std::optional<SingleInstanceCommand> decodeCommand(const QByteArray &payload)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(payload.trimmed(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        markshot::debugLog("single-instance",
                           "invalid ipc command: %s",
                           parseError.errorString().toUtf8().constData());
        return std::nullopt;
    }

    const QJsonObject object = document.object();
    SingleInstanceCommand command;
    command.capture = object.value(QStringLiteral("capture")).toBool(false);
    command.fullscreen = object.value(QStringLiteral("fullscreen")).toBool(false);
    command.allOutputs = object.value(QStringLiteral("allOutputs")).toBool(false);
    command.recordingStatus = object.value(QStringLiteral("recordingStatus")).toBool(false);
    command.stopRecording = object.value(QStringLiteral("stopRecording")).toBool(false);
    command.startRecording = object.value(QStringLiteral("startRecording")).toBool(false);
    command.recordDisplayKey = object.value(QStringLiteral("recordDisplayKey")).toString();
    command.recordGeometryText = object.value(QStringLiteral("recordGeometryText")).toString();
    command.recordOutputPath = object.value(QStringLiteral("recordOutputPath")).toString();
    command.recordFormat = object.value(QStringLiteral("recordFormat")).toString();
    command.recordFps = object.value(QStringLiteral("recordFps")).toInt(15);
    command.recordIncludeAudio = object.value(QStringLiteral("recordIncludeAudio")).toBool(false);
    command.recordDurationMs = object.value(QStringLiteral("recordDurationMs")).toInt(0);
    return command;
}

/**
 * 解码单实例响应。
 * @param payload JSON 载荷。
 * @return 响应内容。
 */
std::optional<SingleInstanceResponse> decodeResponse(const QByteArray &payload)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(payload.trimmed(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return std::nullopt;
    }

    const QJsonObject object = document.object();
    SingleInstanceResponse response;
    response.handled = object.value(QStringLiteral("handled")).toBool(false);
    response.stopped = object.value(QStringLiteral("stopped")).toBool(false);
    response.recordingStarted = object.value(QStringLiteral("recordingStarted")).toBool(false);
    response.message = object.value(QStringLiteral("message")).toString();
    response.recording = recordingStatusFromJson(object.value(QStringLiteral("recording")).toObject());
    return response;
}

/**
 * 编码单实例响应。
 * @param response 响应内容。
 * @return JSON 行。
 */
QByteArray encodeResponse(const SingleInstanceResponse &response)
{
    return QJsonDocument(responseToJsonObject(response)).toJson(QJsonDocument::Compact) + '\n';
}

}  // namespace

QString singleInstanceServerName()
{
    return QStringLiteral("mark-shot-single-instance");
}

bool sendSingleInstanceCommand(const SingleInstanceCommand &command,
                               SingleInstanceResponse *response,
                               QString *error)
{
    if (error) {
        error->clear();
    }

    QLocalSocket socket;
    socket.connectToServer(singleInstanceServerName(), QIODevice::ReadWrite);
    if (!socket.waitForConnected(250)) {
        if (error) {
            *error = socket.errorString();
        }
        return false;
    }

    const QByteArray payload = encodeCommand(command);
    if (socket.write(payload) != payload.size() || !socket.waitForBytesWritten(500)) {
        if (error) {
            *error = socket.errorString();
        }
        return false;
    }

    if (!socket.waitForReadyRead(1000)) {
        if (error) {
            *error = socket.errorString();
        }
        return false;
    }

    QByteArray responseBytes = socket.readAll();
    while (!responseBytes.contains('\n') && socket.waitForReadyRead(100)) {
        responseBytes.append(socket.readAll());
    }
    const std::optional<SingleInstanceResponse> parsed = decodeResponse(responseBytes);
    if (!parsed.has_value()) {
        if (error) {
            *error = QStringLiteral("invalid single-instance response");
        }
        return false;
    }
    if (response) {
        *response = *parsed;
    }
    return true;
}

std::unique_ptr<QLocalServer> listenForSingleInstanceCommands(QString *error)
{
    if (error) {
        error->clear();
    }

    auto server = std::make_unique<QLocalServer>();
    server->setSocketOptions(QLocalServer::UserAccessOption);
    if (server->listen(singleInstanceServerName())) {
        markshot::debugLog("single-instance",
                           "listening on %s",
                           singleInstanceServerName().toUtf8().constData());
        return server;
    }

    if (error) {
        *error = server->errorString();
    }
    return nullptr;
}

void installSingleInstanceCommandHandler(QLocalServer *server,
                                         QObject *context,
                                         SingleInstanceHandler handler)
{
    if (!server || !context || !handler) {
        return;
    }

    QObject::connect(server, &QLocalServer::newConnection, context, [server, handler = std::move(handler)] {
        while (QLocalSocket *socket = server->nextPendingConnection()) {
            QObject::connect(socket, &QLocalSocket::readyRead, socket, [socket, handler] {
                const QByteArray payload = socket->readAll();
                SingleInstanceResponse response;
                if (const std::optional<SingleInstanceCommand> command = decodeCommand(payload)) {
                    response = handler(*command);
                } else {
                    response.message = QStringLiteral("invalid command");
                }
                socket->write(encodeResponse(response));
                socket->flush();
                socket->disconnectFromServer();
            });
            QObject::connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
        }
    });
}

QJsonObject responseToJsonObject(const SingleInstanceResponse &response)
{
    QJsonObject object;
    object.insert(QStringLiteral("handled"), response.handled);
    object.insert(QStringLiteral("stopped"), response.stopped);
    object.insert(QStringLiteral("recordingStarted"), response.recordingStarted);
    object.insert(QStringLiteral("message"), response.message);
    object.insert(QStringLiteral("recording"), recordingStatusToJson(response.recording));
    return object;
}

}  // namespace markshot::ipc
