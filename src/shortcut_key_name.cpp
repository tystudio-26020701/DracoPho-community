#include "shortcut_key_name.h"

namespace markshot::shortcut {

QString qtKeyToShortcutKeyName(int key)
{
    if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        const QChar letter(QLatin1Char(static_cast<char>('a' + key - Qt::Key_A)));
        return QString(letter);
    }
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        const QChar digit(QLatin1Char(static_cast<char>('0' + key - Qt::Key_0)));
        return QString(digit);
    }
    if (key >= Qt::Key_F1 && key <= Qt::Key_F24) {
        return QStringLiteral("F%1").arg(key - Qt::Key_F1 + 1);
    }

    switch (key) {
    case Qt::Key_Backspace: return QStringLiteral("BackSpace");
    case Qt::Key_Tab: return QStringLiteral("Tab");
    case Qt::Key_Return:
    case Qt::Key_Enter: return QStringLiteral("Return");
    case Qt::Key_Escape: return QStringLiteral("Escape");
    case Qt::Key_Space: return QStringLiteral("space");
    case Qt::Key_PageUp: return QStringLiteral("Page_Up");
    case Qt::Key_PageDown: return QStringLiteral("Page_Down");
    case Qt::Key_End: return QStringLiteral("End");
    case Qt::Key_Home: return QStringLiteral("Home");
    case Qt::Key_Left: return QStringLiteral("Left");
    case Qt::Key_Up: return QStringLiteral("Up");
    case Qt::Key_Right: return QStringLiteral("Right");
    case Qt::Key_Down: return QStringLiteral("Down");
    case Qt::Key_Insert: return QStringLiteral("Insert");
    case Qt::Key_Delete: return QStringLiteral("Delete");
    case Qt::Key_Print: return QStringLiteral("Print");
    case Qt::Key_Pause: return QStringLiteral("Pause");
    case Qt::Key_CapsLock: return QStringLiteral("Caps_Lock");
    case Qt::Key_NumLock: return QStringLiteral("Num_Lock");
    case Qt::Key_ScrollLock: return QStringLiteral("Scroll_Lock");
    case Qt::Key_Plus: return QStringLiteral("plus");
    case Qt::Key_Comma: return QStringLiteral("comma");
    case Qt::Key_Minus: return QStringLiteral("minus");
    case Qt::Key_Period: return QStringLiteral("period");
    case Qt::Key_Slash: return QStringLiteral("slash");
    case Qt::Key_Backslash: return QStringLiteral("backslash");
    case Qt::Key_Semicolon: return QStringLiteral("semicolon");
    case Qt::Key_Apostrophe: return QStringLiteral("apostrophe");
    case Qt::Key_BracketLeft: return QStringLiteral("bracketleft");
    case Qt::Key_BracketRight: return QStringLiteral("bracketright");
    case Qt::Key_QuoteLeft: return QStringLiteral("grave");
    default: return {};
    }
}

}  // namespace markshot::shortcut
