#include "screen_capture.h"

#include <QProcess>
#include <QStringList>

namespace {

CaptureResult runGrim(const QStringList &arguments, const QString &outputName)
{
    QProcess grim;
    grim.setProgram(QStringLiteral("grim"));
    grim.setArguments(arguments);
    grim.start(QIODevice::ReadOnly);

    if (!grim.waitForStarted(3000)) {
        return {{}, QStringLiteral("failed to start grim; install grim and run under a Wayland compositor that supports screencopy"), outputName};
    }

    if (!grim.waitForFinished(8000)) {
        grim.kill();
        grim.waitForFinished(1000);
        return {{}, QStringLiteral("grim timed out while capturing the screen"), outputName};
    }

    const QByteArray png = grim.readAllStandardOutput();
    const QByteArray stderrText = grim.readAllStandardError();

    if (grim.exitStatus() != QProcess::NormalExit || grim.exitCode() != 0) {
        QString error = QString::fromLocal8Bit(stderrText).trimmed();
        if (error.isEmpty()) {
            error = QStringLiteral("grim failed with exit code %1").arg(grim.exitCode());
        }
        return {{}, error, outputName};
    }

    QImage image;
    if (!image.loadFromData(png, "PPM") || image.isNull()) {
        return {{}, QStringLiteral("grim returned invalid PPM data"), outputName};
    }

    return {image.convertToFormat(QImage::Format_ARGB32_Premultiplied), {}, outputName};
}

} // namespace

CaptureResult captureScreenFrame(const QString &preferredOutputName, bool allOutputs)
{
    if (!allOutputs && !preferredOutputName.isEmpty()) {
        CaptureResult outputCapture = runGrim({QStringLiteral("-t"), QStringLiteral("ppm"), QStringLiteral("-o"), preferredOutputName, QStringLiteral("-")}, preferredOutputName);
        if (!outputCapture.image.isNull()) {
            return outputCapture;
        }
    }

    return runGrim({QStringLiteral("-t"), QStringLiteral("ppm"), QStringLiteral("-")}, {});
}
