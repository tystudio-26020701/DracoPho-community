#include "history/capture_history_dialog.h"

#include "annotation_launch.h"
#include "clipboard_image.h"
#include "history/capture_history.h"
#include "ui/i18n.h"
#include "ui/icons.h"

#include <QDateTime>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace markshot::capture_history {

namespace {

/// @brief 生成条目的缩略图标与标题文本。
/// @param entry 历史条目。
/// @param icon 输出缩略图图标。
/// @param title 输出标题。
void buildItemContent(const HistoryEntry &entry, QIcon *icon, QString *title)
{
    if (icon) {
        QImageReader reader(entry.path);
        reader.setAutoTransform(true);
        // 只按缩略图尺寸解码，避免 50 张全尺寸 4K 图阻塞主线程。
        const QSize sourceSize = reader.size();
        QSize scaled = QSize(320, 240);
        if (sourceSize.isValid() && !sourceSize.isEmpty()) {
            scaled.scale(sourceSize, Qt::KeepAspectRatio);
        }
        reader.setScaledSize(scaled);
        const QImage image = reader.read();
        if (!image.isNull()) {
            *icon = QIcon(QPixmap::fromImage(image));
        }
    }
    if (title) {
        const QDateTime when = QDateTime::fromMSecsSinceEpoch(entry.timestampMs);
        const QString name = entry.name.isEmpty()
            ? QStringLiteral("—")
            : entry.name;
        *title = QStringLiteral("%1\n%2")
                     .arg(name, when.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    }
}

}  // namespace

CaptureHistoryDialog::CaptureHistoryDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(MS_TR("Capture History"));
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumSize(680, 420);
    resize(780, 500);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    auto *toolbar = new QHBoxLayout;
    toolbar->setSpacing(6);

    m_newCaptureButton = new QPushButton(MS_TR("New Capture"), this);
    m_newCaptureButton->setIcon(markshot::ui::makeToolIcon(ShotWindow::Action::ToolSelect));
    m_newCaptureButton->setToolTip(MS_TR("Start a new screenshot"));
    m_newCaptureButton->setFocusPolicy(Qt::NoFocus);
    connect(m_newCaptureButton, &QPushButton::clicked, this, [this] {
        // 直接关闭（WA_DeleteOnClose 自清理）：否则每次"新建截图"都会遗留
        // 一个隐藏的历史窗口，反复从菜单打开会积累内存。
        close();
        if (m_newCaptureCallback) {
            m_newCaptureCallback();
        }
    });
    toolbar->addWidget(m_newCaptureButton);

    m_clearButton = new QPushButton(MS_TR("Clear All"), this);
    m_clearButton->setFocusPolicy(Qt::NoFocus);
    connect(m_clearButton, &QPushButton::clicked, this, [this] {
        if (QMessageBox::question(this,
                                  MS_TR("Clear All"),
                                  MS_TR("Delete all saved captures from history?"))
            == QMessageBox::Yes) {
            markshot::capture_history::clearCaptures();
            refreshList();
        }
    });
    toolbar->addWidget(m_clearButton);
    toolbar->addStretch(1);

    m_closeButton = new QPushButton(MS_TR("Close"), this);
    m_closeButton->setFocusPolicy(Qt::NoFocus);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);
    toolbar->addWidget(m_closeButton);

    layout->addLayout(toolbar);

    m_list = new QListWidget(this);
    m_list->setViewMode(QListView::IconMode);
    m_list->setIconSize(QSize(160, 120));
    m_list->setResizeMode(QListView::Adjust);
    m_list->setMovement(QListView::Static);
    m_list->setUniformItemSizes(true);
    m_list->setSpacing(10);
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        reeditItem(item);
    });
    connect(m_list, &QListWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QListWidgetItem *item = m_list->itemAt(pos);
        if (!item) {
            return;
        }
        m_list->setCurrentItem(item);
        QMenu menu(m_list);
        QAction *recopy = menu.addAction(MS_TR("Re-copy"));
        QAction *reedit = menu.addAction(MS_TR("Re-edit"));
        QAction *saveAs = menu.addAction(MS_TR("Save As..."));
        menu.addSeparator();
        QAction *remove = menu.addAction(MS_TR("Delete"));
        QAction *chosen = menu.exec(m_list->viewport()->mapToGlobal(pos));
        if (chosen == recopy) {
            recopyItem(item);
        } else if (chosen == reedit) {
            reeditItem(item);
        } else if (chosen == saveAs) {
            saveItemAs(item);
        } else if (chosen == remove) {
            deleteItem(item);
        }
    });
    layout->addWidget(m_list, 1);

    refreshList();
}

void CaptureHistoryDialog::setNewCaptureCallback(NewCaptureCallback callback)
{
    m_newCaptureCallback = std::move(callback);
}

void CaptureHistoryDialog::refreshList()
{
    m_list->clear();
    const QVector<HistoryEntry> entries = markshot::capture_history::listCaptures();
    if (entries.isEmpty()) {
        auto *empty = new QListWidgetItem(m_list);
        empty->setFlags(Qt::NoItemFlags);
        empty->setText(MS_TR("No captures yet — copy, save or pin a screenshot to record it here."));
        empty->setTextAlignment(Qt::AlignCenter);
        return;
    }
    for (const HistoryEntry &entry : entries) {
        QIcon icon;
        QString title;
        buildItemContent(entry, &icon, &title);
        auto *item = new QListWidgetItem(m_list);
        item->setIcon(icon);
        item->setText(title);
        item->setData(Qt::UserRole, entry.id);
        item->setToolTip(QStringLiteral("%1\n%2").arg(title, entry.path));
    }
    m_list->setWordWrap(true);
    m_list->setTextElideMode(Qt::ElideRight);
    m_list->setIconSize(QSize(160, 120));
}

void CaptureHistoryDialog::recopyItem(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    const QString id = item->data(Qt::UserRole).toString();
    for (const HistoryEntry &entry : listCaptures()) {
        if (entry.id == id) {
            QImageReader reader(entry.path);
            reader.setAutoTransform(true);
            const QImage image = reader.read();
            if (image.isNull()) {
                QMessageBox::warning(this,
                                     QStringLiteral("DracoPho"),
                                     MS_TR("Failed to load history image"));
                return;
            }
            markshot::copyImageToClipboard(image);
            break;
        }
    }
}

void CaptureHistoryDialog::reeditItem(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    const QString id = item->data(Qt::UserRole).toString();
    for (const HistoryEntry &entry : listCaptures()) {
        if (entry.id == id) {
            QImageReader reader(entry.path);
            reader.setAutoTransform(true);
            const QImage image = reader.read();
            if (image.isNull()) {
                QMessageBox::warning(this,
                                     QStringLiteral("DracoPho"),
                                     MS_TR("Failed to load history image"));
                return;
            }
            markshot::openImageForAnnotation(image, entry.name);
            break;
        }
    }
}

void CaptureHistoryDialog::saveItemAs(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    const QString id = item->data(Qt::UserRole).toString();
    const QVector<HistoryEntry> entries = listCaptures();
    const auto it = std::find_if(entries.cbegin(), entries.cend(), [&id](const HistoryEntry &entry) {
        return entry.id == id;
    });
    if (it == entries.cend()) {
        return;
    }
    const HistoryEntry entry = *it;

    QImageReader reader(entry.path);
    reader.setAutoTransform(true);
    const QImage image = reader.read();
    if (image.isNull()) {
        QMessageBox::warning(this,
                             QStringLiteral("DracoPho"),
                             MS_TR("Failed to load history image"));
        return;
    }

    auto *dialog = new QFileDialog(this, MS_TR("Save Screenshot"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setFileMode(QFileDialog::AnyFile);
    dialog->setNameFilter(MS_TR("PNG Images (*.png)"));
    dialog->setDefaultSuffix(QStringLiteral("png"));
    dialog->setOption(QFileDialog::DontUseNativeDialog, true);
    const QString baseName = entry.name.isEmpty() ? QStringLiteral("screenshot") : entry.name;
    dialog->selectFile(QDir::home().filePath(baseName + QStringLiteral(".png")));
    connect(dialog, &QFileDialog::fileSelected, this, [this, image](const QString &path) {
        if (!path.isEmpty() && !image.save(path, "PNG")) {
            QMessageBox::warning(this,
                                 QStringLiteral("DracoPho"),
                                 MS_TR("Save failed"));
        }
    });
    dialog->open();
}

void CaptureHistoryDialog::deleteItem(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    const QString id = item->data(Qt::UserRole).toString();
    markshot::capture_history::removeCapture(id);
    refreshList();
}

}  // namespace markshot::capture_history
