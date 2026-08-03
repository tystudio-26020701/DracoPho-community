#include "recording/recording_status.h"

namespace markshot::recording {

QString recordingModeName(RecordingMode mode)
{
    if (mode == RecordingMode::Gif) {
        return QStringLiteral("gif");
    }
    if (mode == RecordingMode::Webp) {
        return QStringLiteral("webp");
    }
    return QStringLiteral("video");
}

}  // namespace markshot::recording
