#include "recording/recording_config_dialog.h"
#include "recording/recording_start_flow.h"
#include "ui/i18n.h"

#include "app_config_store.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardPaths>
#include <QTimer>
#include <QWheelEvent>
#include <QtTest/QtTest>

#include <optional>

namespace {

/**
 * 查找对话框内指定名称的控件。
 * @param dialog 对话框。
 * @param name 控件对象名。
 * @return 控件指针，不存在时返回空指针。
 */
template <typename T>
T *findControl(const markshot::recording::RecordingConfigDialog *dialog, const QString &name)
{
    return dialog->findChild<T *>(name);
}

}  // namespace

class RecordingConfigDialogTest : public QObject {
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
     * 验证视频模式对话框的初始状态与选项映射。
     * @return 无返回值。
     */
    void videoModeDefaults()
    {
        markshot::recording::RecordingConfigDialog dialog(markshot::recording::RecordingMode::Video);
        const markshot::recording::RecordingOptions options = dialog.options();

        QCOMPARE(options.mode, markshot::recording::RecordingMode::Video);
        QVERIFY(options.fps > 0);
        QCOMPARE(options.captureBackend, markshot::recording::RecordingCaptureBackend::Auto);
        QVERIFY(options.outputPath.endsWith(QStringLiteral(".mp4")));
    }

    /**
     * 验证后端下拉框只包含当前平台可用的后端。
     * @return 无返回值。
     */
    void backendOptionsArePlatformAppropriate()
    {
        markshot::recording::RecordingConfigDialog dialog(markshot::recording::RecordingMode::Video);
        auto *backend = findControl<QComboBox>(&dialog, QStringLiteral("recordingBackend"));
        QVERIFY(backend);
        QVERIFY(backend->count() > 0);

        bool hasAuto = false;
        for (int i = 0; i < backend->count(); ++i) {
            const auto backendValue =
                static_cast<markshot::recording::RecordingCaptureBackend>(backend->itemData(i).toInt());
            hasAuto = hasAuto || backendValue == markshot::recording::RecordingCaptureBackend::Auto;
#if defined(_WIN32)
            QVERIFY(backendValue != markshot::recording::RecordingCaptureBackend::Wlroots);
            QVERIFY(backendValue != markshot::recording::RecordingCaptureBackend::PipeWire);
#else
            QVERIFY(backendValue != markshot::recording::RecordingCaptureBackend::WindowsWgc);
#endif
        }
        QVERIFY(hasAuto);
    }

    /**
     * 验证区域录制范围会禁用显示器选择，显示录制范围会启用它。
     * @return 无返回值。
     */
    void scopeControlsDisplayAvailability()
    {
        markshot::recording::RecordingConfigDialog dialog(markshot::recording::RecordingMode::Video);
        auto *scope = findControl<QComboBox>(&dialog, QStringLiteral("recordingScope"));
        auto *display = findControl<QComboBox>(&dialog, QStringLiteral("recordingDisplay"));
        QVERIFY(scope);
        QVERIFY(display);

        scope->setCurrentIndex(
            scope->findData(static_cast<int>(markshot::recording::RecordingScope::Region)));
        QVERIFY(!display->isEnabled());

        scope->setCurrentIndex(
            scope->findData(static_cast<int>(markshot::recording::RecordingScope::Display)));
        QVERIFY(display->isEnabled());
    }

    /**
     * 验证切换录制模式会重建帧率选项并更新输出扩展名。
     * @return 无返回值。
     */
    void switchingModeUpdatesControls()
    {
        markshot::recording::RecordingConfigDialog dialog(markshot::recording::RecordingMode::Video);
        auto *modeSelector = findControl<QComboBox>(&dialog, QStringLiteral("recordingModeSelector"));
        auto *fps = findControl<QComboBox>(&dialog, QStringLiteral("recordingFps"));
        auto *audio = findControl<QCheckBox>(&dialog, QStringLiteral("recordingAudioCheck"));
        auto *outputPath = findControl<QLineEdit>(&dialog, QStringLiteral("recordingOutputPath"));
        QVERIFY(modeSelector);
        QVERIFY(fps);
        QVERIFY(audio);
        QVERIFY(outputPath);

        QVERIFY(outputPath->text().endsWith(QStringLiteral(".mp4")));

        modeSelector->setCurrentIndex(modeSelector->findData(static_cast<int>(markshot::recording::RecordingMode::Gif)));

        QCOMPARE(dialog.options().mode, markshot::recording::RecordingMode::Gif);
        QVERIFY(outputPath->text().endsWith(QStringLiteral(".gif")));
        QVERIFY(!audio->isEnabled());
        QVERIFY(!audio->isChecked());
    }

    /**
     * 验证对话框读取各控件状态到录制选项。
     * @return 无返回值。
     */
    void optionsReflectControlState()
    {
        markshot::recording::RecordingConfigDialog dialog(markshot::recording::RecordingMode::Video);
        auto *fps = findControl<QComboBox>(&dialog, QStringLiteral("recordingFps"));
        auto *audio = findControl<QCheckBox>(&dialog, QStringLiteral("recordingAudioCheck"));
        auto *scope = findControl<QComboBox>(&dialog, QStringLiteral("recordingScope"));
        QVERIFY(fps);
        QVERIFY(audio);
        QVERIFY(scope);

        const int expectedFps = fps->currentData().toInt();
        audio->setChecked(true);
        scope->setCurrentIndex(
            scope->findData(static_cast<int>(markshot::recording::RecordingScope::Region)));

        const markshot::recording::RecordingOptions options = dialog.options();
        QCOMPARE(options.fps, expectedFps);
        QCOMPARE(options.scope, markshot::recording::RecordingScope::Region);
    }

    /**
     * 验证下拉框收到滚轮事件时选中项不被篡改（滚轮防护）。
     * 未聚焦时由对话框级滚轮防护吞掉事件；聚焦时由控件级滚轮抑制器
     * 拦截——与设置窗口行为一致，聚焦也绝不因滚轮改选中项。
     * @return 无返回值。
     */
    void wheelOverComboDoesNotChangeValue()
    {
        markshot::recording::RecordingConfigDialog dialog(markshot::recording::RecordingMode::Video);
        auto *fps = findControl<QComboBox>(&dialog, QStringLiteral("recordingFps"));
        auto *mode = findControl<QComboBox>(&dialog, QStringLiteral("recordingModeSelector"));
        auto *backend = findControl<QComboBox>(&dialog, QStringLiteral("recordingBackend"));
        QVERIFY(fps);
        QVERIFY(mode);
        QVERIFY(backend);

        const QVector<QComboBox *> combos{fps, mode, backend};
        for (const bool focused : {false, true}) {
            for (QComboBox *combo : combos) {
                if (focused) {
                    combo->setFocus(Qt::MouseFocusReason);
                }
                const int before = combo->currentIndex();
                QWheelEvent wheel(QPointF(10, 10),
                                  QPointF(10, 10),
                                  QPoint(),
                                  QPoint(0, 120),
                                  Qt::NoButton,
                                  Qt::NoModifier,
                                  Qt::NoScrollPhase,
                                  false);
                QApplication::sendEvent(combo, &wheel);
                QCOMPARE(combo->currentIndex(), before);
            }
        }
    }

    /**
     * 验证对话框样式跟随配置的界面主题（ui.theme）。
     * @return 无返回值。
     */
    void stylesheetFollowsConfiguredTheme()
    {
        QString error;

        QVERIFY(markshot::writeAppConfigValue(
            QStringList{QStringLiteral("ui"), QStringLiteral("theme")},
            QStringLiteral("dark"),
            &error));
        {
            markshot::recording::RecordingConfigDialog dialog(markshot::recording::RecordingMode::Video);
            QVERIFY(dialog.styleSheet().contains(QStringLiteral("#0F172A")));
        }

        QVERIFY(markshot::writeAppConfigValue(
            QStringList{QStringLiteral("ui"), QStringLiteral("theme")},
            QStringLiteral("light"),
            &error));
        {
            markshot::recording::RecordingConfigDialog dialog(markshot::recording::RecordingMode::Video);
            QVERIFY(dialog.styleSheet().contains(QStringLiteral("#F8FAFC")));
        }
    }

    /**
     * 验证生产录制启动流程与设置窗口一致：非模态打开、WA_DeleteOnClose
     * 自清理、按请求追加置顶。模态窗口在 GNOME 等桌面环境无法最小化，
     * 会导致标题栏最小化按钮失效，因此录制配置窗口必须非模态。
     * @return 无返回值。
     */
    void productionFlowCreatesNonModalWindow()
    {
        std::optional<Qt::WindowFlags> capturedFlags;
        bool dialogWasModal = true;
        bool handled = false;

        QTimer::singleShot(80, this, [&capturedFlags, &dialogWasModal, &handled] {
            const auto tops = QApplication::topLevelWidgets();
            for (QWidget *w : tops) {
                if (auto *dlg = qobject_cast<QDialog *>(w)) {
                    if (dlg->windowFlags() & Qt::WindowStaysOnTopHint) {
                        capturedFlags = dlg->windowFlags();
                        dialogWasModal = dlg->isModal();
                        handled = true;
                        dlg->reject();
                        return;
                    }
                }
            }
        });

        markshot::recording::RecordingStartFlowRequest request;
        request.initialMode = markshot::recording::RecordingMode::Video;
        request.stayOnTop = true;
        request.startDisplayRecording = [](markshot::recording::RecordingOptions) {};
        request.selectRegionRecording = [](markshot::recording::RecordingOptions) {};
        request.showError = [](const QString &) {};
        request.cancelled = [&handled] { handled = true; };

        markshot::recording::runRecordingStartFlow(request);
        QTest::qWait(150);

        QVERIFY(handled);
        QVERIFY(capturedFlags.has_value());
        // 非模态：与设置窗口一致。
        QVERIFY(!dialogWasModal);
        // 悬浮球/托盘入口请求置顶，对话框保持在悬浮球之上。
        QVERIFY((*capturedFlags & Qt::WindowStaysOnTopHint) != 0);
    }

    /**
     * 验证界面字符串随语言设置本地化。
     * @return 无返回值。
     */
    void displaySourcesAreLocalized()
    {
        markshot::i18n::setLanguage(markshot::i18n::Language::Chinese);
        QCOMPARE(markshot::i18n::translate(QStringLiteral("Display %1")), QStringLiteral("显示器 %1"));
        QCOMPARE(markshot::i18n::translate(QStringLiteral("All Displays")), QStringLiteral("所有显示器"));
        QCOMPARE(markshot::i18n::translate(QStringLiteral("wlroots screencopy")), QStringLiteral("wlroots 屏幕复制"));
        QCOMPARE(markshot::i18n::translate(QStringLiteral("Windows Graphics Capture")), QStringLiteral("Windows 图形捕获"));
        QCOMPARE(markshot::i18n::translate(QStringLiteral("Polling")), QStringLiteral("轮询"));
    }
};

QTEST_MAIN(RecordingConfigDialogTest)
#include "recording_config_dialog_test.moc"
