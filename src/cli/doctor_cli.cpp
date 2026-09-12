#include "cli/doctor_cli.h"

#include "app_config_store.h"
#include "cli/window_capture_cli.h"
#include "headless_capture_config.h"
#include "recording/recording_display_source.h"

#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScreen>
#include <QSysInfo>
#include <QTextStream>

#ifdef MARK_SHOT_VERSION
#define DRACOPHO_DOCTOR_VERSION QStringLiteral(MARK_SHOT_VERSION)
#else
#define DRACOPHO_DOCTOR_VERSION QStringLiteral("unknown")
#endif

namespace markshot::cli {
namespace {

QJsonObject environmentObject()
{
    QJsonObject environment;
    environment.insert(QStringLiteral("xdgSessionType"),
                       qEnvironmentVariable("XDG_SESSION_TYPE"));
    environment.insert(QStringLiteral("xdgCurrentDesktop"),
                       qEnvironmentVariable("XDG_CURRENT_DESKTOP"));
    environment.insert(QStringLiteral("waylandDisplay"),
                       qEnvironmentVariable("WAYLAND_DISPLAY"));
    environment.insert(QStringLiteral("display"),
                       qEnvironmentVariable("DISPLAY"));
    return environment;
}

} // namespace

int runDoctor()
{
    QJsonObject root;
    root.insert(QStringLiteral("v"), 1);
    root.insert(QStringLiteral("name"), QStringLiteral("dracoPho"));
    root.insert(QStringLiteral("version"), DRACOPHO_DOCTOR_VERSION);

    QJsonObject qt;
    qt.insert(QStringLiteral("platform"), QGuiApplication::platformName());
    qt.insert(QStringLiteral("runtime"), QString::fromLatin1(qVersion()));
    qt.insert(QStringLiteral("compiled"), QStringLiteral(QT_VERSION_STR));
    root.insert(QStringLiteral("qt"), qt);

    QJsonObject os;
    os.insert(QStringLiteral("product"), QSysInfo::productType());
    os.insert(QStringLiteral("productVersion"), QSysInfo::productVersion());
    os.insert(QStringLiteral("kernel"), QStringLiteral("%1 %2").arg(QSysInfo::kernelType(), QSysInfo::kernelVersion()));
    os.insert(QStringLiteral("architecture"), QSysInfo::currentCpuArchitecture());
    root.insert(QStringLiteral("os"), os);

    root.insert(QStringLiteral("environment"), environmentObject());

    QJsonArray displays;
    const QList<QScreen *> screens = QGuiApplication::screens();
    for (QScreen *screen : screens) {
        if (!screen) {
            continue;
        }
        QJsonObject entry;
        entry.insert(QStringLiteral("name"), screen->name());
        entry.insert(QStringLiteral("key"),
                     markshot::recording::normalizeRecordingDisplayId(screen->name()));
        const QRect geometry = screen->geometry();
        entry.insert(QStringLiteral("x"), geometry.x());
        entry.insert(QStringLiteral("y"), geometry.y());
        entry.insert(QStringLiteral("width"), geometry.width());
        entry.insert(QStringLiteral("height"), geometry.height());
        entry.insert(QStringLiteral("dpr"), screen->devicePixelRatio());
        entry.insert(QStringLiteral("primary"), screen == QGuiApplication::primaryScreen());
        displays.append(entry);
    }
    root.insert(QStringLiteral("displays"), displays);

    const HeadlessCaptureConfig headless =
        headlessCaptureConfigFromRoot(readAppConfigRoot());
    QJsonObject headlessConfig;
    headlessConfig.insert(QStringLiteral("defaultDestination"),
                          headlessCaptureDestinationName(headless.defaultDestination));
    headlessConfig.insert(QStringLiteral("clipboardAllowed"), headless.clipboardAllowed);
    root.insert(QStringLiteral("headless"), headlessConfig);

    root.insert(QStringLiteral("windowDetection"), windowDetectionDiagnostics());

    QJsonArray warnings;
    if (displays.isEmpty()) {
        warnings.append(QStringLiteral("no displays detected; captures will fail "
                                       "(check QT_QPA_PLATFORM and the display server session)"));
    }
    if (!warnings.isEmpty()) {
        root.insert(QStringLiteral("warnings"), warnings);
    }

    QTextStream out(stdout);
    out << QJsonDocument(root).toJson(QJsonDocument::Compact) << '\n';
    out.flush();
    return 0;
}

} // namespace markshot::cli
