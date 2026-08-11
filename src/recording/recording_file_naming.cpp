#include "recording/recording_file_naming.h"

#include "recording/recording_storage_config.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

namespace markshot::recording {
namespace {

/**
 * 展开用户目录缩写。
 * @param path 用户输入路径。
 * @return 展开后的路径。
 */
QString expandHomePath(QString path)
{
    path = path.trimmed();
    if (path == QStringLiteral("~")) {
        return QDir::homePath();
    }
    if (path.startsWith(QStringLiteral("~/"))) {
        return QDir::home().filePath(path.mid(2));
    }
    return path;
}

/**
 * 返回录制模式对应的文件扩展名。
 * @param mode 录制模式。
 * @return 不含点号的扩展名。
 */
QString extensionForMode(RecordingMode mode)
{
    if (mode == RecordingMode::Gif) {
        return QStringLiteral("gif");
    }
    if (mode == RecordingMode::Webp) {
        return QStringLiteral("webp");
    }
    return QStringLiteral("mp4");
}

}  // namespace

QString defaultRecordingPath(RecordingMode mode)
{
    return defaultRecordingPathInDirectory(recordingDirectoryForMode(mode), mode);
}

QString defaultRecordingPathInDirectory(const QString &directory, RecordingMode mode)
{
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"));
    const QString targetDirectory = directory.trimmed().isEmpty()
        ? recordingDirectoryForMode(mode)
        : directory.trimmed();
    QDir().mkpath(targetDirectory);
    return QDir(targetDirectory)
        .filePath(QStringLiteral("dracoPho-recording-%1.%2").arg(timestamp, extensionForMode(mode)));
}

QString normalizedRecordingPath(QString path, RecordingMode mode)
{
    path = expandHomePath(path);
    if (path.isEmpty()) {
        return defaultRecordingPath(mode);
    }

    const QString expected = extensionForMode(mode);
    QFileInfo info(path);
    if (info.suffix().compare(expected, Qt::CaseInsensitive) == 0) {
        return path;
    }
    return QStringLiteral("%1.%2").arg(path, expected);
}

}  // namespace markshot::recording
