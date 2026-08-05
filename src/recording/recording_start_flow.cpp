#include "recording/recording_start_flow.h"

#include "recording/recording_config_dialog.h"
#include "recording/recording_dialog_config.h"
#include "ui/i18n.h"
#include "windows_integration.h"

#include <QApplication>
#include <QDialog>
#include <QGuiApplication>
#include <QIcon>
#include <QMessageBox>
#include <QScreen>

namespace markshot::recording {
namespace {

/**
 * 显示录制启动错误。
 * @param request 启动流程请求。
 * @param message 错误信息。
 * @return 无返回值。
 */
void showStartFlowError(const RecordingStartFlowRequest &request, const QString &message)
{
    if (request.showError) {
        request.showError(message);
        return;
    }
    QMessageBox::warning(request.parent, QStringLiteral("DracoPho"), message);
}

/**
 * 分发对话框结果到请求回调。
 * @param request 启动流程请求。
 * @param dialog 已确认（或已取消）的配置对话框。
 * @param accepted 用户是否点击"开始"。
 * @return 无返回值。
 */
void dispatchResult(const RecordingStartFlowRequest &request,
                    const RecordingConfigDialog *dialog,
                    bool accepted)
{
    if (!accepted) {
        if (request.cancelled) {
            request.cancelled();
        }
        return;
    }

    RecordingOptions options = dialog->options();
    QString saveError;
    saveRecordingDialogConfig(recordingDialogConfigFromOptions(options), &saveError);
    if (options.display.geometry.isEmpty()) {
        showStartFlowError(request, MS_TR("No display is available for recording."));
        return;
    }

    if (options.scope == RecordingScope::Display) {
        options.captureGeometry = options.display.geometry;
        if (request.startDisplayRecording) {
            request.startDisplayRecording(std::move(options));
        }
        return;
    }

    options.scope = RecordingScope::Region;
    if (request.selectRegionRecording) {
        request.selectRegionRecording(std::move(options));
    }
}

}  // namespace

void runRecordingStartFlow(const RecordingStartFlowRequest &request)
{
    // 与设置窗口（showSettingsDialog）一致：非模态、show() 打开、
    // WA_DeleteOnClose 自清理。模态窗口在 GNOME 等桌面环境无法最小化，
    // 会导致标题栏最小化按钮失效；因此不使用 exec()/setWindowModality。
    auto *dialog = new RecordingConfigDialog(request.initialMode, request.parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);

    const QIcon appIcon = QApplication::windowIcon();
    if (!appIcon.isNull()) {
        dialog->setWindowIcon(appIcon);
    }
    // 与设置窗口一致，保留对话框默认窗口标志（含最小化/最大化/关闭按钮），
    // 仅按请求追加置顶（悬浮球/托盘入口需要压在悬浮球之上）。
    if (request.stayOnTop) {
        dialog->setWindowFlag(Qt::WindowStaysOnTopHint, true);
    }
    markshot::windows::setExcludedFromCapture(dialog);

    // 与设置窗口一致，按主屏可用区域手动居中。
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        const QSize hint = dialog->sizeHint();
        const QRect available = screen->availableGeometry();
        dialog->move(available.center() - QPoint(hint.width() / 2, hint.height() / 2));
    }

    QObject::connect(dialog, &QDialog::accepted, dialog, [dialog, request] {
        dispatchResult(request, dialog, true);
    });
    QObject::connect(dialog, &QDialog::rejected, dialog, [dialog, request] {
        dispatchResult(request, dialog, false);
    });

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

}  // namespace markshot::recording
