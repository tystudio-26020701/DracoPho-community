#pragma once

#include <QDialog>

#include <functional>

class QListWidget;
class QListWidgetItem;
class QPushButton;

namespace markshot::capture_history {

/// @brief 截图历史窗口。
///
/// 展示最近捕获的截图缩略图网格，支持重新复制、重新编辑、另存为、
/// 删除与清空；双击条目直接进入图片编辑器。
class CaptureHistoryDialog final : public QDialog {
    Q_OBJECT

public:
    using NewCaptureCallback = std::function<void()>;

    /// @brief 创建截图历史窗口。
    /// @param parent 父控件。
    explicit CaptureHistoryDialog(QWidget *parent = nullptr);

    /// @brief 设置"新建截图"回调（关闭历史窗口并启动一次截图）。
    /// @param callback 新建截图回调。
    void setNewCaptureCallback(NewCaptureCallback callback);

private:
    /// @brief 刷新缩略图列表。
    void refreshList();
    /// @brief 重新复制所选截图到剪贴板。
    /// @param item 列表项。
    void recopyItem(QListWidgetItem *item);
    /// @brief 在图片编辑器中重新打开所选截图。
    /// @param item 列表项。
    void reeditItem(QListWidgetItem *item);
    /// @brief 把所选截图另存为。
    /// @param item 列表项。
    void saveItemAs(QListWidgetItem *item);
    /// @brief 删除所选截图。
    /// @param item 列表项。
    void deleteItem(QListWidgetItem *item);

    QListWidget *m_list = nullptr;
    QPushButton *m_newCaptureButton = nullptr;
    QPushButton *m_clearButton = nullptr;
    QPushButton *m_closeButton = nullptr;
    NewCaptureCallback m_newCaptureCallback;
};

}  // namespace markshot::capture_history
