#pragma once

#include <QAbstractNativeEventFilter>
#include <QKeySequence>
#include <QObject>
#include <QString>

#include <functional>

#if defined(Q_OS_LINUX) && defined(HAVE_XCB)
#include <xcb/xproto.h>
typedef struct _XDisplay Display;
struct xcb_connection_t;
#endif

namespace markshot {

/// @brief 基于 X11 XGrabKey 的全局快捷键实现。
///
/// xdg-desktop-portal 的 GlobalShortcuts 在部分发行版（尤其 Ubuntu/GNOME）
/// 上并不实现或实现不完整，导致全局快捷键不可用。X11/Xorg 会话下使用
/// XGrabKey 直接抓取按键是最可靠的做法，本类在 xcb 平台上注册全局快捷键并
/// 通过原生事件过滤器分发触发回调。
class X11GlobalShortcut final : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT

public:
    struct Shortcut {
        QString id;
        QString description;
        QKeySequence sequence;
        std::function<void()> callback;
    };

    explicit X11GlobalShortcut(QObject *parent = nullptr);
    ~X11GlobalShortcut() override;

    /// @brief 判断当前平台是否支持 X11 全局快捷键。
    /// @return 支持时返回 true。
    static bool isAvailable();

    /// @brief 注册一组全局快捷键。
    /// @param shortcuts 快捷键列表。
    /// @return 全部注册成功时返回 true。
    bool registerShortcuts(const QList<Shortcut> &shortcuts);

    /// @brief 注销全部已注册快捷键。
    /// @return 无返回值。
    void unregisterShortcuts();

    /// @brief 返回最近一次失败的原因。
    /// @return 错误描述。
    QString errorString() const;

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

private:
    struct RegisteredShortcut {
        QString id;
        QKeySequence sequence;
        std::function<void()> callback;
        int keycode = 0;
    };

#if defined(Q_OS_LINUX) && defined(HAVE_XCB)
    /// @brief 键盘映射变更（MappingNotify）后重新解析掩码并重新抓取。
    /// @return 无返回值。
    void remapShortcuts();

    /// @brief 抓取单个快捷键的 4 种锁键变体。
    /// @param entry 快捷键条目。
    /// @return 全部抓取成功时返回 true。
    bool grabShortcutVariants(const RegisteredShortcut &entry);

    Display *m_display = nullptr;
    xcb_connection_t *m_connection = nullptr;
    xcb_window_t m_rootWindow = 0;
    /// @brief 按实际布局解析的 Alt 掩码。
    unsigned int m_altMask = 0;
    /// @brief 按实际布局解析的 Meta/Super 掩码。
    unsigned int m_metaMask = 0;
    /// @brief 按实际布局解析的 NumLock 掩码。
    unsigned int m_numLockMask = 0;
#endif
    QList<RegisteredShortcut> m_shortcuts;
    bool m_installed = false;
    QString m_errorString;
};

}  // namespace markshot
