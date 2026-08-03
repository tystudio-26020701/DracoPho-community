#include "recording/recording_session_manager.h"

#include "notifications/app_notifications.h"
#include "recording/recording_controller.h"
#include "recording/recording_storage_config.h"

#include <QDir>
#include <QFileInfo>

namespace markshot::recording {

RecordingSessionManager &RecordingSessionManager::instance()
{
    static RecordingSessionManager manager;
    return manager;
}

namespace {

/**
 * 清理录制输出目录中残留的 .part.mkv 临时文件（进程被杀 / remux 失败遗留）。
 * @return 无返回值。
 */
void sweepLeftoverTempMkv()
{
    const RecordingStorageConfig config = configuredRecordingStorageConfig();
    const QStringList directories{config.videoDirectory, config.gifDirectory};
    for (const QString &directory : directories) {
        QDir dir(directory);
        if (!dir.exists()) {
            continue;
        }
        const QFileInfoList entries =
            dir.entryInfoList({QStringLiteral("*.part.mkv")}, QDir::Files, QDir::Name);
        for (const QFileInfo &entry : entries) {
            QFile::remove(entry.absoluteFilePath());
        }
    }
}

}  // namespace

RecordingSessionManager::RecordingSessionManager(QObject *parent)
    : QObject(parent)
{
    // 清理上次进程崩溃/remux 失败遗留的临时 MKV，避免磁盘无限增长。
    sweepLeftoverTempMkv();
}

bool RecordingSessionManager::start(const RecordingOptions &options, QObject *parent, QString *error)
{
    if (error) {
        error->clear();
    }
    if (m_controller) {
        if (error) {
            *error = QStringLiteral("recording is already active");
        }
        return false;
    }

    auto *controller = new RecordingController(parent ? parent : this);
    connect(controller, &RecordingController::statusChanged, this, &RecordingSessionManager::statusChanged);
    connect(controller,
            &RecordingController::finished,
            this,
            [this, controller](bool ok, const QString &outputPath, const QString &message) {
                if (m_controller == controller) {
                    m_controller = nullptr;
                }
                // 保留最终结果供 CLI/MCP 查询：结束后的 status() 不再返回空，
                // 而是明确区分成功 / 失败与失败原因。
                m_lastStatus = controller->status();
                m_lastStatus.active = false;
                m_lastStatus.finishedOk = ok;
                m_lastStatus.failed = !ok;
                m_lastStatus.errorMessage = message;
                m_lastStatus.outputPath = outputPath;
                emit statusChanged();
                if (ok) {
                    markshot::notifications::notifyRecordingSaved(outputPath);
                } else {
                    markshot::notifications::notifyRecordingFailed(message);
                }
                emit recordingFinished(ok, outputPath, message);
            });
    if (!controller->start(options, error)) {
        controller->deleteLater();
        return false;
    }

    m_controller = controller;
    m_lastStatus = RecordingStatus();
    emit statusChanged();
    markshot::notifications::notifyRecordingStarted(options);
    return true;
}

bool RecordingSessionManager::stop(QString *error)
{
    if (error) {
        error->clear();
    }
    if (!m_controller) {
        if (error) {
            *error = QStringLiteral("no active recording");
        }
        return false;
    }
    m_controller->requestStop();
    return true;
}

RecordingStatus RecordingSessionManager::status() const
{
    // 无活动录制时返回最近一次结束结果（成功/失败/原因），供 CLI/MCP 判别。
    return m_controller ? m_controller->status() : m_lastStatus;
}

}  // namespace markshot::recording
