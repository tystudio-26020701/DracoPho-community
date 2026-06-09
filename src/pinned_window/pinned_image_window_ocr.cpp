#include "pinned_window/pinned_image_window.h"

#include "clipboard_image.h"
#include "ocr_result.h"
#include "shell_command.h"
#include "ui/i18n.h"

#include <QCursor>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTimer>

namespace markshot::shot {

void PinnedImageWindow::startOcr()
{
    cancelOcr();
    if (!m_config.ocrEnabled) {
        m_translateAfterOcr = false;
        m_copyTextAfterOcr = false;
        return;
    }

    QTemporaryFile tempFile(QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation).isEmpty()
                                ? QDir::tempPath()
                                : QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                                .filePath(QStringLiteral("mark-shot-pin-ocr-XXXXXX.png")));
    tempFile.setAutoRemove(false);
    if (!tempFile.open()) {
        m_translateAfterOcr = false;
        m_copyTextAfterOcr = false;
        return;
    }
    m_ocrTempPath = tempFile.fileName();
    if (!m_pixmap.save(&tempFile, "PNG")) {
        tempFile.close();
        QFile::remove(m_ocrTempPath);
        m_ocrTempPath.clear();
        m_translateAfterOcr = false;
        m_copyTextAfterOcr = false;
        return;
    }
    tempFile.close();

    auto *process = new QProcess(this);
    m_ocrProcess = process;
    if (!m_config.ocrCommand.isEmpty()) {
        QString commandLine = m_config.ocrCommand;
        const bool replaced = replaceExtensionImagePlaceholders(&commandLine, m_ocrTempPath);
        if (!replaced) {
            commandLine += QLatin1Char(' ');
            commandLine += shellQuote(m_ocrTempPath);
        }
        markshot::setShellCommand(process, commandLine);
    } else {
        process->setProgram(defaultOcrHelperProgram());
        process->setArguments({QStringLiteral("--format"),
                               QStringLiteral("json"),
                               QStringLiteral("--backend"),
                               m_config.ocrBackend,
                               m_ocrTempPath});
    }

    connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError error) {
        if (process == m_ocrProcess && process->state() == QProcess::NotRunning) {
            finishOcr(process,
                      QByteArray(),
                      process->readAllStandardError(),
                      -1,
                      QProcess::CrashExit,
                      error);
        }
    });
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        finishOcr(process,
                  process->readAllStandardOutput(),
                  process->readAllStandardError(),
                  exitCode,
                  exitStatus,
                  process->error());
    });
    QTimer::singleShot(m_config.ocrTimeoutMs, process, [process] {
        if (process->state() != QProcess::NotRunning) {
            process->kill();
        }
    });
    process->start();
}

QString PinnedImageWindow::defaultOcrHelperProgram() const
{
    return helperProgramPath(QStringLiteral("mark-shot-ocr"));
}

void PinnedImageWindow::cancelOcr()
{
    if (m_ocrProcess) {
        disconnect(m_ocrProcess, nullptr, this, nullptr);
        if (m_ocrProcess->state() != QProcess::NotRunning) {
            m_ocrProcess->kill();
        }
        m_ocrProcess->deleteLater();
        m_ocrProcess = nullptr;
    }
    if (!m_ocrTempPath.isEmpty()) {
        QFile::remove(m_ocrTempPath);
        m_ocrTempPath.clear();
    }
}

void PinnedImageWindow::finishOcr(QProcess *process,
                                  const QByteArray &output,
                                  const QByteArray &errorOutput,
                                  int exitCode,
                                  QProcess::ExitStatus exitStatus,
                                  QProcess::ProcessError processError)
{
    if (process != m_ocrProcess) {
        return;
    }

    const bool success = exitStatus == QProcess::NormalExit && exitCode == 0;
    if (success && !output.isEmpty()) {
        applyOcrOutput(output, errorOutput);
    } else if (processError == QProcess::FailedToStart && m_config.ocrCommand.isEmpty()) {
        notifyMissingOcrBackend();
        m_translateAfterOcr = false;
        m_copyTextAfterOcr = false;
    } else if (m_config.ocrCommand.isEmpty()
               && ocrOutputReportsMissingBackend(output, errorOutput, m_config.ocrBackend)) {
        notifyMissingOcrBackend();
        m_translateAfterOcr = false;
        m_copyTextAfterOcr = false;
    } else {
        m_translateAfterOcr = false;
        m_copyTextAfterOcr = false;
    }
    m_ocrProcess = nullptr;
    if (!m_ocrTempPath.isEmpty()) {
        QFile::remove(m_ocrTempPath);
        m_ocrTempPath.clear();
    }
    process->deleteLater();
}

void PinnedImageWindow::applyOcrOutput(const QByteArray &output, const QByteArray &errorOutput)
{
    const QVector<OcrToken> tokens = tokensFromJsonOutput(output);
    if (tokens.isEmpty()) {
        if (m_config.ocrCommand.isEmpty()
            && ocrOutputReportsMissingBackend(output, errorOutput, m_config.ocrBackend)) {
            notifyMissingOcrBackend();
        }
        m_translateAfterOcr = false;
        m_copyTextAfterOcr = false;
        return;
    }

    m_ocrTokens = tokens;
    m_translatedTokens.clear();
    m_translationOverlayTokens.clear();
    m_translationActive = false;
    const bool translateAfterOcr = m_translateAfterOcr;
    const bool copyTextAfterOcr = m_copyTextAfterOcr;
    m_translateAfterOcr = false;
    m_copyTextAfterOcr = false;
    updateCursorForPosition(mapFromGlobal(QCursor::pos()));
    if (copyTextAfterOcr) {
        markshot::copyTextToClipboard(allText());
    }
    if (translateAfterOcr) {
        startTranslation(true);
    } else if (m_config.autoTranslateAfterOcr) {
        startTranslation(false, false);
    } else {
        update();
    }
}

void PinnedImageWindow::notifyMissingOcrBackend()
{
    if (m_ocrBackendWarningShown) {
        return;
    }
    m_ocrBackendWarningShown = true;
    sendDesktopNotification(QStringLiteral("Mark Shot"),
                            MS_TR("OCR backend not installed. Install rapidocr or tesseract."));
}

QVector<PinnedImageWindow::OcrToken> PinnedImageWindow::tokensFromJsonOutput(const QByteArray &output) const
{
    QVector<OcrToken> tokens;
    const QVector<markshot::ocr::Token> parsedTokens =
        markshot::ocr::tokensFromJsonOutput(output, m_imageSize);
    tokens.reserve(parsedTokens.size());
    for (const markshot::ocr::Token &parsedToken : parsedTokens) {
        tokens.append({parsedToken.text,
                       parsedToken.imageRect,
                       parsedToken.line,
                       parsedToken.index,
                       parsedToken.confidence});
    }
    return tokens;
}

}  // namespace markshot::shot
