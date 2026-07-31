#include "x11_global_shortcut.h"

#include "shortcut_key_name.h"
#include "ui/i18n.h"
#include "windows_integration.h"

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QtGui/qguiapplication_platform.h>

#if defined(Q_OS_LINUX) && defined(HAVE_XCB)
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#endif

namespace markshot {

namespace {

#if defined(Q_OS_LINUX) && defined(HAVE_XCB)

/// @brief 将 Qt 键值转换为 X11 keysym。
/// @param key Qt 键值。
/// @return 对应的 X11 keysym，无法转换时返回 NoSymbol。
KeySym qtKeyToX11KeySym(int key)
{
    const QString name = markshot::shortcut::qtKeyToShortcutKeyName(key);
    if (name.isEmpty()) {
        return NoSymbol;
    }
    return XStringToKeysym(name.toUtf8().constData());
}

/// @brief 将 Qt 修饰键转换为 X11 修饰键掩码。
/// @param modifiers Qt 修饰键。
/// @return X11 修饰键掩码。
unsigned int qtModifiersToX11(Qt::KeyboardModifiers modifiers)
{
    unsigned int mask = 0;
    if (modifiers.testFlag(Qt::ControlModifier)) {
        mask |= ControlMask;
    }
    if (modifiers.testFlag(Qt::AltModifier)) {
        mask |= Mod1Mask;
    }
    if (modifiers.testFlag(Qt::ShiftModifier)) {
        mask |= ShiftMask;
    }
    if (modifiers.testFlag(Qt::MetaModifier)) {
        mask |= Mod4Mask;
    }
    return mask;
}

#endif

}  // namespace

X11GlobalShortcut::X11GlobalShortcut(QObject *parent)
    : QObject(parent)
{
}

X11GlobalShortcut::~X11GlobalShortcut()
{
    unregisterShortcuts();
}

bool X11GlobalShortcut::isAvailable()
{
#if defined(Q_OS_LINUX) && defined(HAVE_XCB)
    // 仅当会话确为 X11/Xorg 时才启用原生 XGrabKey：XWayland 下 Qt 平台名也是
    // "xcb"，但抓取无法覆盖原生 Wayland 窗口的全局按键。
    if (!markshot::windows::isNativeX11Session()) {
        return false;
    }
    auto *x11 = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    return x11 != nullptr && x11->display() != nullptr && x11->connection() != nullptr;
#else
    return false;
#endif
}

bool X11GlobalShortcut::registerShortcuts(const QList<Shortcut> &shortcuts)
{
    m_errorString.clear();

#if defined(Q_OS_LINUX) && defined(HAVE_XCB)
    if (shortcuts.isEmpty()) {
        m_errorString = MS_TR("No global shortcuts are configured.");
        return false;
    }

    auto *x11 = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    if (!x11 || !x11->display() || !x11->connection()) {
        m_errorString = MS_TR("X11 global shortcut support is not available.");
        return false;
    }
    m_display = x11->display();
    m_connection = x11->connection();

    const xcb_setup_t *setup = xcb_get_setup(m_connection);
    if (!setup) {
        m_errorString = MS_TR("X11 global shortcut support is not available.");
        return false;
    }
    const xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    if (!iter.data) {
        m_errorString = MS_TR("X11 global shortcut support is not available.");
        return false;
    }
    m_rootWindow = iter.data->root;

    QList<RegisteredShortcut> valid;
    for (const Shortcut &shortcut : shortcuts) {
        if (shortcut.id.isEmpty() || !shortcut.callback || shortcut.sequence.isEmpty()) {
            continue;
        }
        const QKeyCombination combination = shortcut.sequence[0];
        const KeySym keysym = qtKeyToX11KeySym(combination.key());
        if (keysym == NoSymbol) {
            continue;
        }
        const int keycode = XKeysymToKeycode(m_display, keysym);
        if (keycode == 0) {
            continue;
        }
        RegisteredShortcut entry;
        entry.id = shortcut.id;
        entry.sequence = shortcut.sequence;
        entry.callback = shortcut.callback;
        entry.keycode = keycode;
        valid.append(entry);
    }
    if (valid.isEmpty()) {
        m_errorString = MS_TR("No global shortcuts are configured.");
        return false;
    }

    // XGrabKey 需要处理 NumLock 与 CapsLock 的状态组合，否则抓取在对应锁键
    // 状态下会失效。常规做法是对 4 种修饰键组合分别抓取。注意：每个快捷键
    // 必须用其自身的修饰键掩码抓取（而不是所有快捷键掩码的并集），否则
    // XGrabKey 要求按键状态精确匹配，混合修饰键的多个快捷键会全部失效。
    bool anyFailed = false;
    for (const RegisteredShortcut &entry : std::as_const(valid)) {
        const unsigned int entryModifiers =
            qtModifiersToX11(entry.sequence[0].keyboardModifiers());
        const unsigned int modifierVariants[] = {
            entryModifiers,
            entryModifiers | Mod2Mask,
            entryModifiers | LockMask,
            entryModifiers | Mod2Mask | LockMask,
        };
        for (unsigned int variant : modifierVariants) {
            const xcb_void_cookie_t cookie =
                xcb_grab_key(m_connection,
                             1,
                             m_rootWindow,
                             static_cast<uint16_t>(variant),
                             static_cast<xcb_keycode_t>(entry.keycode),
                             XCB_GRAB_MODE_ASYNC,
                             XCB_GRAB_MODE_ASYNC);
            const xcb_generic_error_t *error =
                xcb_request_check(m_connection, cookie);
            if (error) {
                std::free(const_cast<xcb_generic_error_t *>(error));
                anyFailed = true;
            }
        }
    }

    m_shortcuts = valid;
    if (!m_installed) {
        qApp->installNativeEventFilter(this);
        m_installed = true;
    }

    if (anyFailed) {
        m_errorString = MS_TR("Failed to register some X11 global shortcuts. "
                              "Another application may already use them.");
        // 撤销已抓取成功的按键并移除事件过滤器，避免调用方（托盘控制器）
        // 走 portal 兜底注册时同一快捷键被 XGrabKey 与 portal 双重处理。
        unregisterShortcuts();
        return false;
    }
    return true;
#else
    m_errorString = MS_TR("X11 global shortcut support is not available.");
    return false;
#endif
}

void X11GlobalShortcut::unregisterShortcuts()
{
#if defined(Q_OS_LINUX) && defined(HAVE_XCB)
    if (m_connection && m_rootWindow) {
        for (const RegisteredShortcut &entry : std::as_const(m_shortcuts)) {
            xcb_ungrab_key(m_connection,
                           static_cast<xcb_keycode_t>(entry.keycode),
                           m_rootWindow,
                           XCB_MOD_MASK_ANY);
        }
        xcb_flush(m_connection);
    }
#endif
    if (m_installed) {
        qApp->removeNativeEventFilter(this);
        m_installed = false;
    }
    m_shortcuts.clear();
}

QString X11GlobalShortcut::errorString() const
{
    return m_errorString;
}

bool X11GlobalShortcut::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(result);

#if defined(Q_OS_LINUX) && defined(HAVE_XCB)
    if (m_shortcuts.isEmpty() || m_connection == nullptr) {
        return false;
    }
    if (eventType != QByteArrayLiteral("xcb_generic_event_t")) {
        return false;
    }

    auto *event = static_cast<xcb_generic_event_t *>(message);
    if (!event) {
        return false;
    }

    if ((event->response_type & ~0x80) != XCB_KEY_PRESS) {
        return false;
    }

    auto *press = reinterpret_cast<xcb_key_press_event_t *>(event);
    // 只比较有效修饰键（Ctrl/Alt/Shift/Logo），忽略 NumLock/CapsLock 锁键状态。
    const unsigned int eventModifiers =
        press->state & (ControlMask | Mod1Mask | ShiftMask | Mod4Mask);

    for (const RegisteredShortcut &entry : std::as_const(m_shortcuts)) {
        if (press->detail != static_cast<uint8_t>(entry.keycode)) {
            continue;
        }
        const unsigned int expected =
            qtModifiersToX11(entry.sequence[0].keyboardModifiers());
        if (eventModifiers == expected) {
            if (entry.callback) {
                entry.callback();
            }
            return true;
        }
    }
#else
    Q_UNUSED(eventType);
    Q_UNUSED(message);
#endif
    return false;
}

}  // namespace markshot
