#include "shot_window_module.h"

using namespace markshot::shot;

/**
 * 对当前选区执行二维码和条形码识别。
 * @return 无返回值。
 */
void ShotWindow::scanCodeSelection()
{
    commitTextEditor();
    if (!hasUsableSelection()) {
        return;
    }

    const QString tempPath = saveSelectionToTempFile();
    if (tempPath.isEmpty()) {
        return;
    }

    const CodeScanConfig config = codeScanConfig();
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QProcess process;
    if (config.command.isEmpty()) {
        process.setProgram(helperProgramPath(QStringLiteral("mark-shot-code-scan")));
        process.setArguments({QStringLiteral("--format"), QStringLiteral("json"), tempPath});
    } else {
        QString commandLine = config.command;
        const bool replaced = replaceExtensionImagePlaceholders(&commandLine, tempPath);
        if (!replaced) {
            commandLine += QLatin1Char(' ');
            commandLine += shellQuote(tempPath);
        }
        markshot::setShellCommand(&process, commandLine);
    }
    process.start();
    if (!process.waitForStarted(3000)) {
        QFile::remove(tempPath);
        QApplication::restoreOverrideCursor();
        showToast(config.command.isEmpty()
                      ? MS_TR("Code scanner helper not found")
                      : MS_TR("Code scan failed"));
        return;
    }
    if (!process.waitForFinished(config.timeoutMs)) {
        process.kill();
        process.waitForFinished(1000);
        QFile::remove(tempPath);
        QApplication::restoreOverrideCursor();
        showToast(MS_TR("Code scan timed out"));
        return;
    }

    QFile::remove(tempPath);
    QApplication::restoreOverrideCursor();

    const QByteArray output = process.readAllStandardOutput();
    const QByteArray errorOutput = process.readAllStandardError();
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        showToast(markshot::code_scan::outputReportsMissingBackend(output, errorOutput)
                      ? MS_TR("Code scanner backend not installed. Install zxing-cpp.")
                      : MS_TR("Code scan failed"));
        return;
    }

    const markshot::code_scan::ParsedOutput parsed = markshot::code_scan::parseOutput(output);
    if (!parsed.validJson) {
        showToast(MS_TR("Code scan failed"));
        return;
    }
    if (parsed.results.isEmpty()) {
        showToast(MS_TR("No code recognized"));
        return;
    }

    auto *window = createCodeScanResultWindow(markshot::code_scan::resultsText(parsed.results));
    window->show();
    window->raise();
    window->activateWindow();
    close();
}
