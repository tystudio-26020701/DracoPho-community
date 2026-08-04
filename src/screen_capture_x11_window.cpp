#include "screen_capture.h"

#include "debug_log.h"

#if defined(HAVE_XCB) && defined(Q_OS_LINUX) && defined(HAVE_XCOMPOSITE)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>
#endif

namespace {

#if defined(HAVE_XCB) && defined(Q_OS_LINUX) && defined(HAVE_XCOMPOSITE)

/// @brief 吞掉合成重定向期间的异步 X 错误（如窗口已被合成器重定向时的 BadAccess），
/// 避免崩溃退出。注意：吞错只用于防止进程崩溃，最终成功与否以调用返回值判定。
/// @param display X 连接。
/// @param error 错误信息。
/// @return 固定返回 0。
int swallowXError(Display *display, XErrorEvent *error)
{
    Q_UNUSED(display);
    Q_UNUSED(error);
    return 0;
}

/// @brief 判断当前 X 会话是否运行合成管理器（_NET_WM_CM_S<screen> 选择权持有者）。
/// @param display X 连接。
/// @return 有合成管理器时返回 true。
bool compositingManagerRunning(Display *display)
{
    if (!display) {
        return false;
    }
    int eventBase = 0;
    int errorBase = 0;
    if (!XCompositeQueryExtension(display, &eventBase, &errorBase)) {
        return false;
    }
    const int screen = DefaultScreen(display);
    const QByteArray selectionName =
        QByteArray("_NET_WM_CM_S") + QByteArray::number(screen);
    const Atom manager = XInternAtom(display, selectionName.constData(), False);
    return XGetSelectionOwner(display, manager) != None;
}

/// @brief 将合成命名 pixmap 读入 QImage。
/// @param display X 连接。
/// @param pixmap 合成命名 pixmap。
/// @return 读取成功时返回图像，失败返回空图像。
QImage readX11PixmapIntoImage(Display *display, Pixmap pixmap)
{
    if (!display || pixmap == None) {
        return {};
    }

    Window root = None;
    int x = 0;
    int y = 0;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int border = 0;
    unsigned int depth = 0;
    if (!XGetGeometry(display, pixmap, &root, &x, &y, &width, &height, &border, &depth)) {
        return {};
    }
    if (width == 0 || height == 0 || width > 16384 || height > 16384) {
        return {};
    }

    XImage *image = XGetImage(display, pixmap, 0, 0, width, height, AllPlanes, ZPixmap);
    if (!image) {
        return {};
    }

    QImage::Format format = (depth >= 32)
        ? QImage::Format_ARGB32
        : QImage::Format_RGB32;
    const QImage view(reinterpret_cast<const uchar *>(image->data),
                      static_cast<int>(width),
                      static_cast<int>(height),
                      image->bytes_per_line,
                      format);
    QImage copy = view.copy();
    XDestroyImage(image);
    if (format == QImage::Format_ARGB32) {
        copy = copy.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }
    return copy;
}

#endif

}  // namespace

QImage captureX11WindowContent(qulonglong windowId, QString *error)
{
#if defined(HAVE_XCB) && defined(Q_OS_LINUX) && defined(HAVE_XCOMPOSITE)
    if (windowId == 0) {
        if (error) {
            *error = QStringLiteral("invalid X11 window id");
        }
        return {};
    }

    Display *display = XOpenDisplay(nullptr);
    if (!display) {
        if (error) {
            *error = QStringLiteral("cannot open the X11 display for window capture");
        }
        return {};
    }

    int eventBase = 0;
    int errorBase = 0;
    if (!XCompositeQueryExtension(display, &eventBase, &errorBase)) {
        XCloseDisplay(display);
        if (error) {
            *error = QStringLiteral("XComposite extension is not available on this X11 server");
        }
        return {};
    }

    const Window window = static_cast<Window>(windowId);
    int (*previousHandler)(Display *, XErrorEvent *) = XSetErrorHandler(swallowXError);

    // 校验窗口仍然存在：枚举与抓取之间窗口可能已销毁，其 XID 会被服务器立即
    // 复用给另一个窗口。先做一次属性往返，BadWindow 时直接失败，而不是吞掉
    // 错误后静默读到另一个窗口的合成内容。
    XWindowAttributes attributes;
    if (!XGetWindowAttributes(display, window, &attributes)) {
        XSetErrorHandler(previousHandler);
        XCloseDisplay(display);
        if (error) {
            *error = QStringLiteral("the target X11 window no longer exists");
        }
        return {};
    }

    // 无合成管理器时临时把窗口重定向到离屏存储，这样即使窗口被遮挡/最小化，
    // 命名 pixmap 也能保留其最后渲染内容；已有合成器时窗口通常已自动重定向，
    // 此时再重定向会触发 BadAccess（被吞掉），因此跳过。后续请求均为往返，
    // 天然与前面的请求保持顺序，无需额外 XSync。
    const bool compositorRunning = compositingManagerRunning(display);
    bool redirectedHere = false;
    if (!compositorRunning) {
        XCompositeRedirectWindow(display, window, CompositeRedirectAutomatic);
        redirectedHere = true;
    }

    QImage image;
    const Pixmap pixmap = XCompositeNameWindowPixmap(display, window);
    if (pixmap != None) {
        image = readX11PixmapIntoImage(display, pixmap);
        XFreePixmap(display, pixmap);
    }

    if (redirectedHere) {
        // 只有我们主动重定向的窗口才恢复；若窗口已被外部销毁，Unredirect 的
        // BadWindow 由吞错处理器吸收，不影响返回结果。
        XCompositeUnredirectWindow(display, window, CompositeRedirectAutomatic);
    }

    XSetErrorHandler(previousHandler);
    XCloseDisplay(display);

    if (image.isNull() && error) {
        *error = QStringLiteral(
            "the window has no retained content (minimized before first exposure, "
            "or the compositor dropped its buffer)");
    }
    return image;
#else
    Q_UNUSED(windowId);
    if (error) {
        *error = QStringLiteral("X11 window-object capture is not available in this build");
    }
    return {};
#endif
}
