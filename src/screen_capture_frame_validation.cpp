#include "screen_capture_frame_validation.h"

#include <QSize>

#include <cstdlib>

namespace markshot {

bool isSuspiciousSolidFrame(const QImage &image)
{
    if (image.isNull() || image.width() < 2 || image.height() < 2) {
        return false;
    }

    // 降采样到 8x8 再统计颜色多样性：真实桌面（窗口、图标、文字、渐变壁纸）
    // 缩略后必然出现多种颜色；合成器返回的纯色占位帧缩略后仍是单色。
    const QImage thumb =
        image.scaled(QSize(8, 8), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QRgb first = 0;
    bool haveFirst = false;
    for (int y = 0; y < thumb.height(); ++y) {
        for (int x = 0; x < thumb.width(); ++x) {
            const QRgb px = thumb.pixel(x, y);
            if (!haveFirst) {
                first = px;
                haveFirst = true;
                continue;
            }
            // 任一通道差超过 8 即视为有内容，不是纯色占位帧。
            if (std::abs(qRed(px) - qRed(first)) > 8
                || std::abs(qGreen(px) - qGreen(first)) > 8
                || std::abs(qBlue(px) - qBlue(first)) > 8) {
                return false;
            }
        }
    }
    return true;
}

}  // namespace markshot
