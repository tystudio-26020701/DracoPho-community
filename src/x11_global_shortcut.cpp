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
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#endif

namespace markshot {

namespace {

#if defined(Q_OS_LINUX) && defined(HAVE_XCB)

/// @brief 解析实际 X11 修饰键掩码。
///
/// Alt/Super/Logo 等修饰键并不总是落在固定的 Mod1/Mod4 位（例如
/// altwin:swap_alt_win 会把 Alt 键映射为 Super_L，或自定义 xmodmap 把
/// Alt 放到 Mod5）。硬编码掩码会导致快捷键在这些布局上静默失效。SOTA
/// 做法是按 keysym 在修饰键映射表中解析实际掩码：
/// 遍历 XGetModifierMapping 返回的 8 个修饰位，对每个按键码取其基级
/// （level 0）keysym，命中 Alt/Super/Meta/Hyper/NumLock 即记录该修饰位。
struct X11ModifierMasks {
    /// @brief Alt 键实际掩码（默认 Mod1Mask）。
    unsigned int alt = Mod1Mask;
    /// @brief Super/Logo 键实际掩码（默认 Mod4Mask）。
    unsigned int super = Mod4Mask;
    /// @brief Meta 键实际掩码（默认 Mod4Mask）。
    unsigned int meta = Mod4Mask;
    /// @brief Num_Lock 键实际掩码（默认 Mod2Mask）。
    unsigned int numLock = Mod2Mask;
};

/// @brief 检查 keysym 是否为指定按键族。
/// @param keysym 待检查的 keysym。
/// @param sym 目标 keysym。
/// @return 命中（含左右手变体）时返回 true。
bool isKeySymVariant(KeySym keysym, KeySym sym)
{
    return keysym == sym || keysym == sym + 1;  // XK_Alt_L + 1 == XK_Alt_R
}

/// @brief 取指定键码在基础层（level 0）的 keysym，XKB 不可用时回退核心协议。
/// @param display X11 Display。
/// @param keycode 按键码。
/// @return keysym，无法解析时返回 NoSymbol。
KeySym baseKeySymForKeycode(Display *display, KeyCode keycode)
{
    if (!display || keycode == 0) {
        return NoSymbol;
    }
    KeySym keysym = XkbKeycodeToKeysym(display, keycode, 0, 0);
    if (keysym != NoSymbol) {
        return keysym;
    }
    // XkbKeycodeToKeysym 在 XKB 扩展不可用时返回 NoSymbol，退回核心协议接口。
    // XKeycodeToKeysym 已废弃但仍可用，这里仅作 XKB 缺失时的兜底。
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    const KeySym fallback = XKeycodeToKeysym(display, keycode, 0);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    return fallback;
}

/// @brief 从修饰键映射表解析 Alt/Super/Meta/NumLock 的实际掩码。
/// @param display X11 Display。
/// @return 解析结果；无法解析时保留默认值。
X11ModifierMasks resolveX11ModifierMasks(Display *display)
{
    static const unsigned int kModifierMasks[8] = {
        ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask,
    };

    X11ModifierMasks result;
    if (!display) {
        return result;
    }

    XModifierKeymap *modmap = XGetModifierMapping(display);
    if (!modmap) {
        return result;
    }

    bool foundAlt = false;
    bool foundSuper = false;
    bool foundMeta = false;
    bool foundNumLock = false;
    for (int modifierIndex = 0; modifierIndex < 8; ++modifierIndex) {
        for (int keyIndex = 0; keyIndex < modmap->max_keypermod; ++keyIndex) {
            const KeyCode keycode =
                modmap->modifiermap[modifierIndex * modmap->max_keypermod + keyIndex];
            if (keycode == 0) {
                continue;
            }
            // level 0 = 基础层 keysym（不随 Shift/Caps 变化），适合识别物理键。
            const KeySym keysym = baseKeySymForKeycode(display, keycode);
            if (!foundAlt && (isKeySymVariant(keysym, XK_Alt_L) || keysym == XK_Meta_L
                              || keysym == XK_Meta_R)) {
                result.alt = kModifierMasks[modifierIndex];
                foundAlt = true;
            }
            if (!foundSuper && (isKeySymVariant(keysym, XK_Super_L))) {
                result.super = kModifierMasks[modifierIndex];
                foundSuper = true;
            }
            if (!foundMeta && (keysym == XK_Meta_L || keysym == XK_Meta_R)) {
                result.meta = kModifierMasks[modifierIndex];
                foundMeta = true;
            }
            if (!foundNumLock && keysym == XK_Num_Lock) {
                result.numLock = kModifierMasks[modifierIndex];
                foundNumLock = true;
            }
        }
    }
    XFreeModifiermap(modmap);

    // 大多数键盘布局 Alt 与 Meta 是同一物理键：Alt 未单独解析时沿用 Meta，
    // Meta 未单独解析时沿用 Super/Alt，保证组合键都能生成有效掩码。
    if (!foundAlt && foundMeta) {
        result.alt = result.meta;
    }
    if (!foundAlt && foundSuper) {
        result.alt = result.super;
    }
    if (!foundMeta && foundSuper) {
        result.meta = result.super;
    }
    if (!foundMeta && foundAlt) {
        result.meta = result.alt;
    }

    return result;
}

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

/// @brief 将 Qt 修饰键转换为 X11 修饰键掩码（按实际键盘布局解析）。
/// @param modifiers Qt 修饰键。
/// @param altMask Alt 键实际掩码。
/// @param metaMask Meta/Super 键实际掩码。
/// @return X11 修饰键掩码。
unsigned int qtModifiersToX11(Qt::KeyboardModifiers modifiers, unsigned int altMask, unsigned int metaMask)
{
    unsigned int mask = 0;
    if (modifiers.testFlag(Qt::ControlModifier)) {
        mask |= ControlMask;
    }
    if (modifiers.testFlag(Qt::AltModifier)) {
        mask |= altMask;
    }
    if (modifiers.testFlag(Qt::ShiftModifier)) {
        mask |= ShiftMask;
    }
    if (modifiers.testFlag(Qt::MetaModifier)) {
        mask |= metaMask;
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
    // NumLock 掩码按实际布局解析（部分系统 NumLock 不在 Mod2 位）。
    const X11ModifierMasks masks = resolveX11ModifierMasks(m_display);
    m_altMask = masks.alt;
    m_metaMask = masks.meta;
    m_numLockMask = masks.numLock;

    bool anyFailed = false;
    for (const RegisteredShortcut &entry : std::as_const(valid)) {
        if (!grabShortcutVariants(entry)) {
            anyFailed = true;
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

#if defined(Q_OS_LINUX) && defined(HAVE_XCB)

bool X11GlobalShortcut::grabShortcutVariants(const RegisteredShortcut &entry)
{
    if (!m_connection || !m_rootWindow) {
        return false;
    }

    const unsigned int entryModifiers =
        qtModifiersToX11(entry.sequence[0].keyboardModifiers(), m_altMask, m_metaMask);
    const unsigned int modifierVariants[] = {
        entryModifiers,
        entryModifiers | m_numLockMask,
        entryModifiers | LockMask,
        entryModifiers | m_numLockMask | LockMask,
    };

    bool allGrabbed = true;
    for (unsigned int variant : modifierVariants) {
        const xcb_void_cookie_t cookie =
            xcb_grab_key(m_connection,
                         1,
                         m_rootWindow,
                         static_cast<uint16_t>(variant),
                         static_cast<xcb_keycode_t>(entry.keycode),
                         XCB_GRAB_MODE_ASYNC,
                         XCB_GRAB_MODE_ASYNC);
        const xcb_generic_error_t *error = xcb_request_check(m_connection, cookie);
        if (error) {
            std::free(const_cast<xcb_generic_error_t *>(error));
            allGrabbed = false;
        }
    }
    return allGrabbed;
}

void X11GlobalShortcut::remapShortcuts()
{
    if (!m_connection || !m_rootWindow || m_shortcuts.isEmpty()) {
        return;
    }

    // 键盘映射变化（布局切换/xmodmap）后 keycode 与修饰键掩码都可能改变：
    // 先按新映射重新解析掩码，再重算 keycode 并重新抓取，保证快捷键继续生效。
    const X11ModifierMasks masks = resolveX11ModifierMasks(m_display);
    m_altMask = masks.alt;
    m_metaMask = masks.meta;
    m_numLockMask = masks.numLock;

    for (RegisteredShortcut &entry : m_shortcuts) {
        xcb_ungrab_key(m_connection,
                       static_cast<xcb_keycode_t>(entry.keycode),
                       m_rootWindow,
                       XCB_MOD_MASK_ANY);
        const KeySym keysym = qtKeyToX11KeySym(entry.sequence[0].key());
        entry.keycode = keysym != NoSymbol ? XKeysymToKeycode(m_display, keysym) : 0;
        if (entry.keycode != 0) {
            grabShortcutVariants(entry);
        }
    }
    xcb_flush(m_connection);
}

#endif

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

    // 键盘映射变更（切换键盘布局、xmodmap 修改）时 keycode 与修饰键掩码
    // 都会变化：重新解析掩码并重新抓取，保证快捷键继续生效（SOTA 实践）。
    if ((event->response_type & ~0x80) == XCB_MAPPING_NOTIFY) {
        remapShortcuts();
        return false;
    }

    if ((event->response_type & ~0x80) != XCB_KEY_PRESS) {
        return false;
    }

    auto *press = reinterpret_cast<xcb_key_press_event_t *>(event);
    // 只比较有效修饰键（Ctrl/Alt/Shift/Logo），忽略 NumLock/CapsLock 锁键状态。
    // 使用按实际布局解析的掩码，避免 Alt/Super 互换等自定义布局下快捷键失效。
    const unsigned int eventModifiers =
        press->state & (ControlMask | m_altMask | ShiftMask | m_metaMask);

    for (const RegisteredShortcut &entry : std::as_const(m_shortcuts)) {
        if (press->detail != static_cast<uint8_t>(entry.keycode)) {
            continue;
        }
        const unsigned int expected =
            qtModifiersToX11(entry.sequence[0].keyboardModifiers(), m_altMask, m_metaMask);
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
