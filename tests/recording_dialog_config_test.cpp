#include "recording/recording_dialog_config.h"
#include "recording/recording_file_naming.h"
#include "recording/recording_status.h"

#include <QDir>
#include <QFileInfo>
#include <QtTest/QtTest>

namespace {

/**
 * 生成平台可用的临时目录文件路径。
 * @param fileName 文件名。
 * @return 临时目录下的完整文件路径。
 */
QString temporaryFilePath(const QString &fileName)
{
    return QDir::temp().filePath(fileName);
}

/**
 * 读取文件路径所在目录的绝对路径。
 * @param path 文件路径。
 * @return 文件所在目录的绝对路径。
 */
QString absoluteDirectoryPath(const QString &path)
{
    return QFileInfo(path).absolutePath();
}

}  // namespace

class RecordingDialogConfigTest : public QObject {
    Q_OBJECT

private slots:
    /**
     * 隔离应用配置路径，避免测试读写用户真实配置。
     * @return 无返回值。
     */
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    /**
     * 验证录制启动界面配置可以从 JSON 恢复。
     * @return 无返回值。
     */
    void readsDialogConfigFromRoot()
    {
        const QString inputPath =
            temporaryFilePath(QStringLiteral("mark-shot-test.mp4"));
        QJsonObject dialog;
        dialog.insert(QStringLiteral("mode"), QStringLiteral("video"));
        dialog.insert(QStringLiteral("scope"), QStringLiteral("display"));
        dialog.insert(QStringLiteral("backend"), QStringLiteral("wlroots-screencopy"));
        dialog.insert(QStringLiteral("fps"), 48);
        dialog.insert(QStringLiteral("includeAudio"), true);
        dialog.insert(QStringLiteral("outputPath"), inputPath);
        dialog.insert(QStringLiteral("displayKey"), QStringLiteral("screen:DP-1"));

        QJsonObject recording;
        recording.insert(QStringLiteral("dialog"), dialog);
        QJsonObject root;
        root.insert(QStringLiteral("recording"), recording);

        const markshot::recording::RecordingDialogConfig config =
            markshot::recording::recordingDialogConfigFromRoot(
                root,
                markshot::recording::RecordingMode::Video);

        QCOMPARE(config.mode, markshot::recording::RecordingMode::Video);
        QCOMPARE(config.scope, markshot::recording::RecordingScope::Display);
        QCOMPARE(config.backend, markshot::recording::RecordingCaptureBackend::Wlroots);
        QCOMPARE(config.fps, 48);
        QCOMPARE(config.videoFps, 48);
        QCOMPARE(config.gifFps, 12);
        QVERIFY(config.includeAudio);
        QCOMPARE(absoluteDirectoryPath(config.outputPath), absoluteDirectoryPath(inputPath));
        QVERIFY(config.outputPath.endsWith(QStringLiteral(".mp4")));
        QVERIFY(config.outputPath != inputPath);
        QCOMPARE(config.displayKey, QStringLiteral("screen:DP-1"));
    }

    /**
     * 验证旧版完整输出路径会按路由模式迁移为目录并重新生成文件名。
     * @return 无返回值。
     */
    void migratesLegacyCompoundOutputPathToDirectory()
    {
        const QString inputPath =
            temporaryFilePath(QStringLiteral("mark-shot-recording-20260704-220015.mp4.gif"));
        QJsonObject dialog;
        dialog.insert(QStringLiteral("mode"), QStringLiteral("gif"));
        dialog.insert(QStringLiteral("outputPath"), inputPath);

        QJsonObject recording;
        recording.insert(QStringLiteral("dialog"), dialog);
        QJsonObject root;
        root.insert(QStringLiteral("recording"), recording);

        const markshot::recording::RecordingDialogConfig config =
            markshot::recording::recordingDialogConfigFromRoot(
                root,
                markshot::recording::RecordingMode::Video);

        QCOMPARE(config.mode, markshot::recording::RecordingMode::Video);
        QCOMPARE(absoluteDirectoryPath(config.outputPath), absoluteDirectoryPath(inputPath));
        QVERIFY(config.outputPath.endsWith(QStringLiteral(".mp4")));
        QVERIFY(!config.outputPath.endsWith(QStringLiteral(".mp4.gif")));
        QVERIFY(config.outputPath != inputPath);
    }

    /**
     * 验证 GIF 和视频录制帧率从独立配置键读取。
     * @return 无返回值。
     */
    void readsSeparateFrameRatesForRecordingModes()
    {
        QJsonObject dialog;
        dialog.insert(QStringLiteral("fps"), 10);
        dialog.insert(QStringLiteral("mode"), QStringLiteral("gif"));
        dialog.insert(QStringLiteral("videoFps"), 60);
        dialog.insert(QStringLiteral("gifFps"), 12);

        QJsonObject recording;
        recording.insert(QStringLiteral("dialog"), dialog);
        QJsonObject root;
        root.insert(QStringLiteral("recording"), recording);

        const markshot::recording::RecordingDialogConfig videoConfig =
            markshot::recording::recordingDialogConfigFromRoot(
                root,
                markshot::recording::RecordingMode::Video);
        const markshot::recording::RecordingDialogConfig gifConfig =
            markshot::recording::recordingDialogConfigFromRoot(
                root,
                markshot::recording::RecordingMode::Gif);

        QCOMPARE(videoConfig.fps, 60);
        QCOMPARE(videoConfig.videoFps, 60);
        QCOMPARE(videoConfig.gifFps, 12);
        QCOMPARE(gifConfig.fps, 12);
        QCOMPARE(gifConfig.videoFps, 60);
        QCOMPARE(gifConfig.gifFps, 12);
    }

    /**
     * 验证旧版单帧率只迁移到旧模式对应的帧率状态。
     * @return 无返回值。
     */
    void migratesLegacyFrameRateByLegacyMode()
    {
        QJsonObject dialog;
        dialog.insert(QStringLiteral("mode"), QStringLiteral("gif"));
        dialog.insert(QStringLiteral("fps"), 24);

        QJsonObject recording;
        recording.insert(QStringLiteral("dialog"), dialog);
        QJsonObject root;
        root.insert(QStringLiteral("recording"), recording);

        const markshot::recording::RecordingDialogConfig videoConfig =
            markshot::recording::recordingDialogConfigFromRoot(
                root,
                markshot::recording::RecordingMode::Video);
        const markshot::recording::RecordingDialogConfig gifConfig =
            markshot::recording::recordingDialogConfigFromRoot(
                root,
                markshot::recording::RecordingMode::Gif);

        QCOMPARE(videoConfig.fps, 30);
        QCOMPARE(videoConfig.videoFps, 30);
        QCOMPARE(videoConfig.gifFps, 24);
        QCOMPARE(gifConfig.fps, 24);
        QCOMPARE(gifConfig.videoFps, 30);
        QCOMPARE(gifConfig.gifFps, 24);
    }

    /**
     * 验证录制选项可以转为启动界面持久化配置。
     * @return 无返回值。
     */
    void createsDialogConfigFromOptions()
    {
        const QString inputPath = temporaryFilePath(QStringLiteral("sample.gif"));
        markshot::recording::RecordingOptions options;
        options.mode = markshot::recording::RecordingMode::Gif;
        options.scope = markshot::recording::RecordingScope::Region;
        options.captureBackend = markshot::recording::RecordingCaptureBackend::PipeWire;
        options.fps = 12;
        options.includeAudio = false;
        options.outputPath = inputPath;
        options.display.screenName = QStringLiteral("HDMI-A-1");

        const markshot::recording::RecordingDialogConfig config =
            markshot::recording::recordingDialogConfigFromOptions(options);

        QCOMPARE(config.mode, markshot::recording::RecordingMode::Gif);
        QCOMPARE(config.scope, markshot::recording::RecordingScope::Region);
        QCOMPARE(config.backend, markshot::recording::RecordingCaptureBackend::PipeWire);
        QCOMPARE(config.fps, 12);
        QCOMPARE(config.videoFps, 30);
        QCOMPARE(config.gifFps, 12);
        QVERIFY(!config.includeAudio);
        QCOMPARE(absoluteDirectoryPath(config.outputPath), absoluteDirectoryPath(inputPath));
        QVERIFY(config.outputPath.endsWith(QStringLiteral(".gif")));
        QVERIFY(config.outputPath != inputPath);
        QCOMPARE(config.displayKey, QStringLiteral("screen:HDMI-A-1"));
    }

    /**
     * 验证切换到另一个录制模式后开始录制不会丢失该模式的帧率选择：
     * 两个模式的独立帧率都从录制选项透传到持久化配置。
     * @return 无返回值。
     */
    void preservesPerModeFrameRatesAcrossModeSwitch()
    {
        markshot::recording::RecordingOptions options;
        options.mode = markshot::recording::RecordingMode::Video;
        options.fps = 48;
        options.videoFps = 48;
        options.gifFps = 15;

        const markshot::recording::RecordingDialogConfig config =
            markshot::recording::recordingDialogConfigFromOptions(options);

        QCOMPARE(config.mode, markshot::recording::RecordingMode::Video);
        QCOMPARE(config.fps, 48);
        QCOMPARE(config.videoFps, 48);
        QCOMPARE(config.gifFps, 15);

        // 回读：以 GIF 模式打开对话框时仍能还原 15fps。
        QJsonObject dialog;
        dialog.insert(QStringLiteral("gifFps"), config.gifFps);
        dialog.insert(QStringLiteral("videoFps"), config.videoFps);
        QJsonObject recording;
        recording.insert(QStringLiteral("dialog"), dialog);
        QJsonObject root;
        root.insert(QStringLiteral("recording"), recording);

        const markshot::recording::RecordingDialogConfig gifConfig =
            markshot::recording::recordingDialogConfigFromRoot(
                root,
                markshot::recording::RecordingMode::Gif);
        QCOMPARE(gifConfig.gifFps, 15);
    }

    /**
     * 验证 WebP 动图模式的配置往返：模式、帧率（复用 gifFps）与扩展名。
     * @return 无返回值。
     */
    void webpModeRoundTrip()
    {
        QJsonObject dialog;
        dialog.insert(QStringLiteral("gifFps"), 15);
        dialog.insert(QStringLiteral("videoFps"), 30);
        QJsonObject recording;
        recording.insert(QStringLiteral("dialog"), dialog);
        QJsonObject root;
        root.insert(QStringLiteral("recording"), recording);

        const markshot::recording::RecordingDialogConfig config =
            markshot::recording::recordingDialogConfigFromRoot(
                root,
                markshot::recording::RecordingMode::Webp);

        QCOMPARE(config.mode, markshot::recording::RecordingMode::Webp);
        QCOMPARE(config.fps, 15);

        markshot::recording::RecordingOptions options;
        options.mode = markshot::recording::RecordingMode::Webp;
        options.fps = 12;
        options.videoFps = 30;
        options.gifFps = 12;
        const markshot::recording::RecordingDialogConfig fromOptions =
            markshot::recording::recordingDialogConfigFromOptions(options);
        QCOMPARE(fromOptions.mode, markshot::recording::RecordingMode::Webp);
        QCOMPARE(fromOptions.gifFps, 12);

        QVERIFY(markshot::recording::defaultRecordingPath(markshot::recording::RecordingMode::Webp)
                    .endsWith(QStringLiteral(".webp")));
        QCOMPARE(markshot::recording::recordingModeName(markshot::recording::RecordingMode::Webp),
                 QStringLiteral("webp"));
    }

    /**
     * 验证保存时同时持久化两个模式的帧率：以 Video 模式保存（GIF 帧率已在
     * 会话内改成 15）后，下次以 GIF 模式打开仍能还原 15，而不是回退旧值。
     * @return 无返回值。
     */
    void saveRoundTripPersistsBothFrameRates()
    {
        markshot::recording::RecordingDialogConfig config;
        config.mode = markshot::recording::RecordingMode::Video;
        config.scope = markshot::recording::RecordingScope::Region;
        config.backend = markshot::recording::RecordingCaptureBackend::Auto;
        config.fps = 48;
        config.videoFps = 48;
        config.gifFps = 15;
        config.outputPath = temporaryFilePath(QStringLiteral("mark-shot-save.mp4"));

        QString saveError;
        QVERIFY2(markshot::recording::saveRecordingDialogConfig(config, &saveError),
                 qPrintable(saveError));

        const markshot::recording::RecordingDialogConfig videoConfig =
            markshot::recording::configuredRecordingDialogConfig(
                markshot::recording::RecordingMode::Video);
        QCOMPARE(videoConfig.videoFps, 48);

        const markshot::recording::RecordingDialogConfig gifConfig =
            markshot::recording::configuredRecordingDialogConfig(
                markshot::recording::RecordingMode::Gif);
        QCOMPARE(gifConfig.gifFps, 15);
    }
};

QTEST_APPLESS_MAIN(RecordingDialogConfigTest)
#include "recording_dialog_config_test.moc"
