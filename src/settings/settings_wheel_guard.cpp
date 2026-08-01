#include "settings/settings_wheel_guard.h"

#include <QAbstractSlider>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QEvent>
#include <QScrollArea>
#include <QWidget>
#include <QWheelEvent>

namespace markshot::settings {
namespace {

/// @brief 设置对话框滚轮事件过滤器。
///
/// Qt 默认行为下，滚轮事件会送达鼠标下的控件；QComboBox 与 QAbstractSpinBox
/// 即使未获得键盘焦点也会响应滚轮并修改内容（选中项/数值），用户在设置页
/// 上下滚动翻页时极易误改配置。本过滤器把这类"未聚焦"控件的滚轮事件转发
/// 给最近的 QScrollArea 视口，让页面按用户意图滚动；找不到滚动区域时直接
/// 吞掉事件，保证控件的值绝不会被悬停滚动篡改。控件聚焦时仍保留滚轮调整
/// 值的能力。
class WheelGuard final : public QObject {
public:
    explicit WheelGuard(QWidget *dialog)
        : QObject(dialog)
        , m_dialog(dialog)
    {
        QApplication::instance()->installEventFilter(this);
    }

    ~WheelGuard() override
    {
        if (QCoreApplication *app = QCoreApplication::instance()) {
            app->removeEventFilter(this);
        }
    }

    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() != QEvent::Wheel || !m_dialog) {
            return QObject::eventFilter(watched, event);
        }

        auto *widget = qobject_cast<QWidget *>(watched);
        if (!widget || !isInDialogWindow(widget)) {
            return QObject::eventFilter(watched, event);
        }

        QWidget *control = guardedControl(widget);
        if (!control || controlHasFocus(control)) {
            return QObject::eventFilter(watched, event);
        }

        auto *wheelEvent = static_cast<QWheelEvent *>(event);
        // 转发给外层滚动区域，保证设置页仍可滚动翻页；找不到滚动区域时
        // 也吞掉事件，杜绝悬停滚动篡改控件值。
        if (!redirectToScrollArea(control, wheelEvent)) {
            event->accept();
        }
        return true;
    }

private:
    /// @brief 判断控件是否属于设置对话框所在顶层窗口。
    /// @param widget 事件目标控件。
    /// @return 属于对话框时返回 true。
    bool isInDialogWindow(QWidget *widget) const
    {
        QWidget *window = widget->window();
        return window && m_dialog && window == m_dialog->window();
    }

    /// @brief 从事件目标向上查找应受防护的控件。
    /// @param widget 事件目标控件。
    /// @return 找到的下拉框/数值框/滑块，未找到时返回空指针。
    static QWidget *guardedControl(QWidget *widget)
    {
        for (QWidget *current = widget; current; current = current->parentWidget()) {
            if (qobject_cast<QAbstractSpinBox *>(current) || qobject_cast<QComboBox *>(current)
                || qobject_cast<QAbstractSlider *>(current)) {
                return current;
            }
            // 到达滚动区域仍未命中，说明悬停在普通控件上，无需防护。
            if (qobject_cast<QScrollArea *>(current)) {
                return nullptr;
            }
        }
        return nullptr;
    }

    /// @brief 判断控件或其子控件是否持有键盘焦点。
    /// @param control 下拉框/数值框。
    /// @return 持有焦点时返回 true。
    static bool controlHasFocus(QWidget *control)
    {
        const QWidget *focus = QApplication::focusWidget();
        return focus && (control == focus || control->isAncestorOf(focus));
    }

    /// @brief 把滚轮事件转发给控件所属的外层滚动区域视口。
    /// @param control 下拉框/数值框。
    /// @param event 原始滚轮事件。
    /// @return 找到滚动区域并转发成功时返回 true。
    static bool redirectToScrollArea(QWidget *control, QWheelEvent *event)
    {
        for (QWidget *current = control; current; current = current->parentWidget()) {
            if (auto *area = qobject_cast<QScrollArea *>(current)) {
                QApplication::sendEvent(area->viewport(), event);
                return true;
            }
        }
        return false;
    }

    QWidget *m_dialog = nullptr;
};

}  // namespace

QObject *installSettingsWheelGuard(QWidget *dialog)
{
    return new WheelGuard(dialog);
}

}  // namespace markshot::settings
