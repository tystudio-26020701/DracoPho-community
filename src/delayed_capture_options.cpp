#include "delayed_capture_options.h"

#include "ui/i18n.h"

namespace markshot {

QString delayedCapturePresetLabel(int seconds)
{
    return MS_TR("%1 Second(s)").arg(seconds);
}

QStringList delayedCapturePresetLabels()
{
    QStringList labels;
    labels.reserve(static_cast<int>(kDelayedCapturePresets.size()));
    for (const int seconds : kDelayedCapturePresets) {
        labels.append(delayedCapturePresetLabel(seconds));
    }
    return labels;
}

}  // namespace markshot
