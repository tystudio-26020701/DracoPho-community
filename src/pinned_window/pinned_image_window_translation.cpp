#include "pinned_window/pinned_image_window.h"

#include "ocr_result.h"
#include "shell_command.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextDocument>
#include <QTextOption>
#include <QTimer>

#include <algorithm>

namespace markshot::shot {

void PinnedImageWindow::startTranslation(bool activateWhenFinished, bool showBusyCursor)
{
    if (m_ocrTokens.isEmpty()) {
        return;
    }
    cancelTranslation();
    clearTextSelection();
    m_activateTranslationWhenFinished = activateWhenFinished;

    QTemporaryFile inputFile(QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation).isEmpty()
                                 ? QDir::tempPath()
                                 : QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                                 .filePath(QStringLiteral("mark-shot-translate-XXXXXX.json")));
    inputFile.setAutoRemove(false);
    if (!inputFile.open()) {
        m_activateTranslationWhenFinished = true;
        return;
    }
    m_translationInputPath = inputFile.fileName();
    inputFile.write(translationInputJson());
    inputFile.close();

    auto *process = new QProcess(this);
    m_translationProcess = process;
    if (!m_config.translationCommand.isEmpty()) {
        QString commandLine = m_config.translationCommand;
        bool replaced = false;
        replaceShellPlaceholder(&commandLine, QStringLiteral("{input}"), m_translationInputPath, &replaced);
        replaceShellPlaceholder(&commandLine, QStringLiteral("{inputPath}"), m_translationInputPath, &replaced);
        replaceShellPlaceholder(&commandLine, QStringLiteral("{targetLanguage}"), m_config.translationTargetLanguage, &replaced);
        replaceShellPlaceholder(&commandLine, QStringLiteral("{config}"), appConfigPath(), &replaced);
        if (!replaced) {
            commandLine += QLatin1Char(' ');
            commandLine += shellQuote(m_translationInputPath);
        }
        markshot::setShellCommand(process, commandLine);
    } else {
        process->setProgram(defaultTranslationHelperProgram());
        process->setArguments({QStringLiteral("--input"),
                               m_translationInputPath,
                               QStringLiteral("--target-language"),
                               m_config.translationTargetLanguage,
                               QStringLiteral("--config"),
                               appConfigPath()});
    }

    connect(process, &QProcess::errorOccurred, this, [this, process] {
        if (process == m_translationProcess && process->state() == QProcess::NotRunning) {
            finishTranslation(process, QByteArray());
        }
    });
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        const QByteArray output = exitStatus == QProcess::NormalExit && exitCode == 0
            ? process->readAllStandardOutput()
            : QByteArray();
        finishTranslation(process, output);
    });
    QTimer::singleShot(m_config.translationTimeoutMs, process, [process] {
        if (process->state() != QProcess::NotRunning) {
            process->kill();
        }
    });
    if (showBusyCursor) {
        setTranslationBusyCursor(true);
    }
    process->start();
    update();
}

QByteArray PinnedImageWindow::translationInputJson() const
{
    QJsonArray tokens;
    for (const OcrToken &token : m_ocrTokens) {
        QJsonObject object;
        object.insert(QStringLiteral("text"), token.text);
        object.insert(QStringLiteral("box"), rectToJson(token.imageRect));
        object.insert(QStringLiteral("line"), token.line);
        object.insert(QStringLiteral("index"), token.index);
        object.insert(QStringLiteral("confidence"), token.confidence);
        tokens.append(object);
    }
    QJsonObject root;
    root.insert(QStringLiteral("targetLanguage"), m_config.translationTargetLanguage);
    root.insert(QStringLiteral("tokens"), tokens);
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

QJsonArray PinnedImageWindow::rectToJson(QRectF rect) const
{
    return markshot::ocr::rectToJson(rect);
}

QString PinnedImageWindow::defaultTranslationHelperProgram() const
{
    return helperProgramPath(QStringLiteral("mark-shot-translate"));
}

void PinnedImageWindow::cancelTranslation()
{
    if (m_translationProcess) {
        disconnect(m_translationProcess, nullptr, this, nullptr);
        if (m_translationProcess->state() != QProcess::NotRunning) {
            m_translationProcess->kill();
        }
        m_translationProcess->deleteLater();
        m_translationProcess = nullptr;
    }
    if (!m_translationInputPath.isEmpty()) {
        QFile::remove(m_translationInputPath);
        m_translationInputPath.clear();
    }
    m_activateTranslationWhenFinished = true;
    setTranslationBusyCursor(false);
}

void PinnedImageWindow::finishTranslation(QProcess *process, const QByteArray &output)
{
    if (process != m_translationProcess) {
        return;
    }
    if (!output.isEmpty()) {
        const QVector<OcrToken> tokens = tokensFromJsonOutput(output);
        if (!tokens.isEmpty()) {
            m_translationOverlayTokens = tokens;
            m_translatedTokens = selectableTranslationTokens(tokens);
            m_translationActive = m_activateTranslationWhenFinished;
            clearTextSelection();
            updateCursorForPosition(mapFromGlobal(QCursor::pos()));
            update();
        }
    }
    m_translationProcess = nullptr;
    if (!m_translationInputPath.isEmpty()) {
        QFile::remove(m_translationInputPath);
        m_translationInputPath.clear();
    }
    setTranslationBusyCursor(false);
    m_activateTranslationWhenFinished = true;
    process->deleteLater();
}

bool PinnedImageWindow::canRequestTranslation() const
{
    return m_config.ocrEnabled && !m_translationProcess;
}

void PinnedImageWindow::requestTranslation()
{
    if (!canRequestTranslation()) {
        return;
    }
    if (!m_translationOverlayTokens.isEmpty()) {
        setTranslationActive(true);
        return;
    }
    if (m_ocrTokens.isEmpty()) {
        m_translateAfterOcr = true;
        if (!m_ocrProcess) {
            startOcr();
        }
        return;
    }
    m_translateAfterOcr = false;
    startTranslation(true);
}

void PinnedImageWindow::setTranslationActive(bool active)
{
    if (active && m_translationOverlayTokens.isEmpty()) {
        return;
    }
    m_translationActive = active;
    clearTextSelection();
    updateCursorForPosition(mapFromGlobal(QCursor::pos()));
    update();
}

void PinnedImageWindow::setTranslationBusyCursor(bool active)
{
    if (m_translationBusyCursor == active) {
        return;
    }
    m_translationBusyCursor = active;
    if (active) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
    } else {
        QApplication::restoreOverrideCursor();
        updateCursorForPosition(mapFromGlobal(QCursor::pos()));
    }
}

QVector<PinnedImageWindow::OcrToken>
PinnedImageWindow::selectableTranslationTokens(const QVector<OcrToken> &tokens) const
{
    QVector<OcrToken> selectableTokens;
    int selectableIndex = 0;
    for (const OcrToken &token : tokens) {
        const QVector<OcrToken> splitTokens = splitTokenForSelection(token);
        for (OcrToken splitToken : splitTokens) {
            splitToken.index = selectableIndex++;
            selectableTokens.append(splitToken);
        }
    }
    return selectableTokens;
}

QVector<PinnedImageWindow::OcrToken> PinnedImageWindow::splitTokenForSelection(const OcrToken &token) const
{
    QVector<OcrToken> splitTokens;
    if (token.text.size() <= 1) {
        splitTokens.append(token);
        return splitTokens;
    }

    qreal totalWeight = 0.0;
    QVector<qreal> weights;
    weights.reserve(token.text.size());
    for (const QChar ch : token.text) {
        const qreal weight = selectionCharacterWeight(ch);
        weights.append(weight);
        totalWeight += weight;
    }
    if (totalWeight <= 0.0 || token.imageRect.width() <= 0.0) {
        splitTokens.append(token);
        return splitTokens;
    }

    qreal offset = 0.0;
    for (int i = 0; i < token.text.size(); ++i) {
        const qreal nextOffset = offset + token.imageRect.width() * weights.at(i) / totalWeight;
        OcrToken splitToken = token;
        splitToken.text = token.text.mid(i, 1);
        splitToken.imageRect = QRectF(token.imageRect.left() + offset,
                                      token.imageRect.top(),
                                      std::max<qreal>(1.0, nextOffset - offset),
                                      token.imageRect.height());
        splitTokens.append(splitToken);
        offset = nextOffset;
    }
    return splitTokens;
}

qreal PinnedImageWindow::selectionCharacterWeight(QChar ch) const
{
    if (ch.isSpace()) {
        return 0.45;
    }
    if (markshot::ocr::isNoLeadingSpacePunctuation(ch)) {
        return 0.65;
    }
    if (ch.unicode() < 0x80) {
        return 0.75;
    }
    return 1.0;
}

void PinnedImageWindow::drawTranslationOverlay(QPainter &painter) const
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    for (const OcrToken &token : m_translationOverlayTokens) {
        const QRectF textRect = imageToWidget(token.imageRect).adjusted(-3.0, -2.0, 3.0, 2.0);
        if (textRect.isEmpty()) {
            continue;
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 232));
        painter.drawRoundedRect(textRect, 2.0, 2.0);
        QFont font = painter.font();
        const int pixelSize = std::clamp(qRound(textRect.height() * 0.62), 8, 28);
        font.setPixelSize(pixelSize);
        QTextDocument document;
        document.setDefaultFont(font);
        document.setDocumentMargin(1.0);
        document.setTextWidth(textRect.width());
        QTextOption option;
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        document.setDefaultTextOption(option);
        document.setPlainText(token.text);
        painter.save();
        painter.setClipRect(textRect);
        painter.translate(textRect.topLeft());
        document.drawContents(&painter, QRectF(QPointF(0.0, 0.0), textRect.size()));
        painter.restore();
    }
    painter.restore();
}

}  // namespace markshot::shot
