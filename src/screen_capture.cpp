#include "screen_capture.h"

#include <QGuiApplication>
#include <QPixmap>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRect>
#include <QScreen>
#include <QStringList>

namespace {

bool isWaylandSession()
{
    const QString sessionType = QProcessEnvironment::systemEnvironment().value(QStringLiteral("XDG_SESSION_TYPE")).toLower();
    return sessionType == QStringLiteral("wayland");
}

CaptureResult captureWithQScreen(const CaptureRequest &request)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return {{}, QStringLiteral("no screen available for capture"), {}, {}};
    }

    QPixmap pixmap = screen->grabWindow(0);
    if (pixmap.isNull()) {
        return {{}, QStringLiteral("QScreen::grabWindow returned null pixmap"), {}, {}};
    }

    QImage image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);

    if (request.sourceGeometry.isValid() && !request.sourceGeometry.isEmpty()) {
        const QRect geo = request.sourceGeometry.normalized().intersected(image.rect());
        if (!geo.isEmpty()) {
            image = image.copy(geo);
        }
    }

    return {image, {}, {}, request.sourceGeometry};
}

QString grimGeometry(QRect geometry)
{
    geometry = geometry.normalized();
    return QStringLiteral("%1,%2 %3x%4")
        .arg(geometry.x())
        .arg(geometry.y())
        .arg(geometry.width())
        .arg(geometry.height());
}

CaptureResult runGrim(const QStringList &arguments, const QString &outputName, QRect sourceGeometry)
{
    QProcess grim;
    grim.setProgram(QStringLiteral("grim"));
    grim.setArguments(arguments);
    grim.start(QIODevice::ReadOnly);

    if (!grim.waitForStarted(3000)) {
        return {{}, QStringLiteral("failed to start grim; install grim and run under a Wayland compositor that supports screencopy"), outputName, sourceGeometry};
    }

    if (!grim.waitForFinished(8000)) {
        grim.kill();
        grim.waitForFinished(1000);
        return {{}, QStringLiteral("grim timed out while capturing the screen"), outputName, sourceGeometry};
    }

    const QByteArray png = grim.readAllStandardOutput();
    const QByteArray stderrText = grim.readAllStandardError();

    if (grim.exitStatus() != QProcess::NormalExit || grim.exitCode() != 0) {
        QString error = QString::fromLocal8Bit(stderrText).trimmed();
        if (error.isEmpty()) {
            error = QStringLiteral("grim failed with exit code %1").arg(grim.exitCode());
        }
        return {{}, error, outputName, sourceGeometry};
    }

    QImage image;
    if (!image.loadFromData(png, "PPM") || image.isNull()) {
        return {{}, QStringLiteral("grim returned invalid PPM data"), outputName, sourceGeometry};
    }

    return {image.convertToFormat(QImage::Format_ARGB32_Premultiplied), {}, outputName, sourceGeometry};
}

} // namespace

CaptureResult captureScreenFrame(const CaptureRequest &request)
{
    if (!isWaylandSession()) {
        return captureWithQScreen(request);
    }

    const QStringList baseArguments{QStringLiteral("-t"), QStringLiteral("ppm"), QStringLiteral("-s"), QStringLiteral("1")};

    if (request.sourceGeometry.isValid() && !request.sourceGeometry.isEmpty()) {
        QStringList arguments = baseArguments;
        arguments << QStringLiteral("-g") << grimGeometry(request.sourceGeometry) << QStringLiteral("-");
        CaptureResult geometryCapture = runGrim(arguments, request.allOutputs ? QString() : request.preferredOutputName, request.sourceGeometry);
        if (!geometryCapture.image.isNull() || request.allOutputs || request.preferredOutputName.isEmpty()) {
            return geometryCapture;
        }
    }

    if (!request.allOutputs && !request.preferredOutputName.isEmpty()) {
        QStringList arguments = baseArguments;
        arguments << QStringLiteral("-o") << request.preferredOutputName << QStringLiteral("-");
        return runGrim(arguments, request.preferredOutputName, {});
    }

    QStringList arguments = baseArguments;
    arguments << QStringLiteral("-");
    return runGrim(arguments, {}, {});
}
