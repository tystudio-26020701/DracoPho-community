#pragma once

#include <QImage>
#include <QString>

namespace markshot {

bool copyImageToClipboard(const QImage &image);
bool copyTextToClipboard(const QString &text);

// Headless-safe variant used by the CLI / MCP server. It never creates a
// transient QClipboard source: on Wayland and X11 it spawns a persistent
// owner process (wl-copy / xclip) so a short-lived process can copy an image
// without blocking on compositor clipboard round-trips and without losing the
// content when it exits. On Windows it falls back to QClipboard::setImage.
bool copyImageToClipboardHeadless(const QImage &image);

} // namespace markshot
