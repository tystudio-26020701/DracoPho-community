#pragma once

#include <QRect>
#include <QString>
#include <QVector>
#include <optional>

namespace markshot {

struct WindowInfo {
    QRect rect;
    std::optional<int> zOrder;
    // Identity fields filled by detection backends that provide them. They are
    // used by the headless window/component capture CLI and the MCP server to
    // let callers select windows by id, title, class or instance.
    QString id;        // stable identifier (X11 hex id, or a backend-provided id)
    QString title;     // window title (may be empty)
    QString className; // WM_CLASS class / app id
    QString instance;  // WM_CLASS instance / app name
    QString monitor;   // monitor name when the backend reports it
    QString workspace; // workspace / desktop when the backend reports it
};

QString markShotConfigDir();
QString appConfigPath();
bool ensureAppConfigFile();
bool windowDetectionEnabled();

QVector<WindowInfo> collectConfiguredWindowInfos(const QRect &captureGeometry,
                                                 const QString &outputName,
                                                 bool allOutputs);

} // namespace markshot
