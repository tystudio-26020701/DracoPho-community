#pragma once

#include <QAbstractNativeEventFilter>
#include <QKeySequence>
#include <QList>
#include <QObject>
#include <QString>

#include "recording/recording_options.h"

#include <functional>

class QApplication;
class QAction;
class QMenu;
class QSystemTrayIcon;
class QTimer;

namespace markshot {

class GlobalShortcutPortal;
class X11GlobalShortcut;

class WindowsTrayController final : public QObject, public QAbstractNativeEventFilter {
public:
    struct Config {
#if defined(Q_OS_WIN)
        bool autoStart = true;
#else
        bool autoStart = false;
#endif
        bool hotkeysEnabled = true;
        QKeySequence captureHotkey = QKeySequence(QStringLiteral("Ctrl+Alt+S"));
        QKeySequence fullscreenHotkey;
    };

    using Callback = std::function<void()>;
    using TimedCaptureCallback = std::function<void(int seconds)>;
    using RecordingRegionCallback = std::function<void(recording::RecordingOptions)>;
    using FloatingBallVisibility = std::function<bool()>;

    /// @brief 创建托盘控制器。
    /// @param application 应用实例。
    /// @param config 托盘配置。
    /// @param parent 父对象。
    /// @param showTrayIcon 是否显示托盘图标。为 false 时仅注册全局快捷键
    /// （悬浮球-only 组合下仍需热键常驻，但用户未选择托盘）。
    explicit WindowsTrayController(QApplication *application,
                                   Config config,
                                   QObject *parent = nullptr,
                                   bool showTrayIcon = true);
    ~WindowsTrayController() override;

    static bool hotkeysSupported();
    static Config readConfig();

    void setCaptureCallbacks(Callback capture, Callback fullscreen);
    void setRecordingRegionCallback(RecordingRegionCallback callback);
    /// @brief 关联"延时截图"回调（秒数入参）。设置后托盘菜单出现延时截图子菜单。
    /// @param callback 延时截图回调。
    void setTimedCaptureCallback(TimedCaptureCallback callback);

    /// @brief 关联悬浮球的"显示/隐藏"开关。
    ///
    /// 托盘菜单会添加"显示/隐藏悬浮球"项：点击调用 toggle 回调，菜单显示前
    /// 通过 visible 回调刷新文案。未设置时不显示该菜单项。
    /// @param toggle 切换悬浮球可见状态的回调。
    /// @param visible 返回悬浮球当前是否可见的回调。
    void setFloatingBallVisibilityControl(Callback toggle, FloatingBallVisibility visible);

    bool start();
    QString errorString() const;

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

private:
    void triggerCapture();
    void triggerFullscreenCapture();

    /**
     * 从托盘菜单请求开始录制。
     * @return 无返回值。
     */
    void startRecordingFromTray();

    /**
     * 从托盘菜单请求停止当前录制。
     * @return 无返回值。
     */
    void stopRecordingFromTray();

    /**
     * 刷新托盘中的录制状态和停止动作。
     * @return 无返回值。
     */
    void updateRecordingState();

    /**
     * 语言切换后重新翻译菜单与提示文案。
     * @return 无返回值。
     */
    void retranslateUi();
    void registerHotkeys();
    void unregisterHotkeys();

    QApplication *m_application = nullptr;
    Config m_config;
    Callback m_captureCallback;
    Callback m_fullscreenCaptureCallback;
    RecordingRegionCallback m_recordingRegionCallback;
    TimedCaptureCallback m_timedCaptureCallback;
    Callback m_floatingBallToggle;
    FloatingBallVisibility m_floatingBallVisible;
    QMenu *m_menu = nullptr;
    QSystemTrayIcon *m_tray = nullptr;
    QAction *m_captureAction = nullptr;
    QAction *m_fullscreenAction = nullptr;
    QAction *m_settingsAction = nullptr;
    QMenu *m_delayedCaptureMenu = nullptr;
    QList<QAction *> m_delayedCaptureItems;
    QAction *m_startRecordingAction = nullptr;
    QAction *m_recordingStatusAction = nullptr;
    QAction *m_stopRecordingAction = nullptr;
    QAction *m_quitAction = nullptr;
    QAction *m_floatingBallToggleAction = nullptr;
    QTimer *m_recordingStatusTimer = nullptr;
    QString m_errorString;
    bool m_showTrayIcon = true;
    bool m_nativeEventFilterInstalled = false;
    bool m_captureHotkeyRegistered = false;
    bool m_fullscreenHotkeyRegistered = false;
    GlobalShortcutPortal *m_globalShortcutPortal = nullptr;
    X11GlobalShortcut *m_x11GlobalShortcut = nullptr;
};

}  // namespace markshot
