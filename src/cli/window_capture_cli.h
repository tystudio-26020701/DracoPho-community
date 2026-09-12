#pragma once

#include <QCommandLineParser>
#include <QJsonObject>

namespace markshot::cli {

// Registers the headless window/component capture options (--list-windows,
// --window, --window-by, --capture-destination) on the given parser.
void addWindowCaptureOptions(QCommandLineParser *parser);

// Runs the headless window/component capture flow when --list-windows,
// --window or --capture-destination is present. Returns an application exit
// code, or -1 when no window capture option is set and normal startup should
// continue.
int runWindowCaptureIfRequested(const QCommandLineParser &parser);

// 返回窗口检测链路诊断（platform/source/count），供 --doctor 汇总。
QJsonObject windowDetectionDiagnostics();

} // namespace markshot::cli
