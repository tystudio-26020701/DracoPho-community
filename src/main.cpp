#include "screen_capture.h"
#include "shot_window.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("mark-shot"));
    QApplication::setApplicationDisplayName(QStringLiteral("Mark Shot"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.1"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Wayland screenshot selection and annotation tool for niri."));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption allOutputsOption(QStringLiteral("all-outputs"), QStringLiteral("Capture all outputs instead of the current Qt screen."));
    QCommandLineOption xdgWindowOption(QStringLiteral("xdg-window"), QStringLiteral("Use a regular fullscreen xdg window instead of layer-shell."));
    QCommandLineOption fullscreenAnnotationOption({QStringLiteral("fullscreen"), QStringLiteral("full-screen")},
                                                  QStringLiteral("Skip region selection and annotate the full captured frame."));
    parser.addOption(allOutputsOption);
    parser.addOption(xdgWindowOption);
    parser.addOption(fullscreenAnnotationOption);
    parser.process(app);

    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }

    const bool allOutputs = parser.isSet(allOutputsOption);
    const QString outputName = (!allOutputs && screen) ? screen->name() : QString();
    CaptureResult capture = captureScreenFrame(outputName, allOutputs);
    if (capture.image.isNull()) {
        QMessageBox::critical(nullptr, QStringLiteral("Mark Shot"), capture.error);
        return 1;
    }

    ShotWindow *window = new ShotWindow(capture.image, capture.outputName);
    if (screen && !allOutputs) {
        window->setScreen(screen);
    }

    const bool useRegularWindow = parser.isSet(xdgWindowOption);
    const bool layerShellReady = !useRegularWindow && window->configureLayerShell(screen);
    if (layerShellReady) {
        window->show();
    } else {
        window->showFullScreen();
        window->raise();
        window->activateWindow();
    }
    if (parser.isSet(fullscreenAnnotationOption)) {
        window->startFullscreenAnnotation();
    }

    return QApplication::exec();
}
